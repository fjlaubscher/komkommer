#import <Foundation/Foundation.h>
#import <MediaPlayer/MediaPlayer.h>

#include "media_keys.h"

#include <cstring>
#include <dlfcn.h>

/*
**	The system hands the media keys to whichever process owns the now playing slot, so a
**	game only sees them after nominating itself. MediaPlayer has no public call for that,
**	so the private MediaRemote entry point does it. When the framework or symbol is
**	missing the claim reports failure and the hardware key events keep driving the score.
*/
namespace
{

using SetCanBeNowPlayingApplication = void (*)(unsigned char);

SetCanBeNowPlayingApplication Claim;

bool Installed;
bool Enabled;
bool Claimed;
char Title[64];

/*
**	Returns false when the private call is unavailable, so the caller knows the system did
**	not hand over the keys.
*/
bool Claim_Now_Playing(bool active)
{
    if (Claim == nullptr) {
        void* framework = dlopen("/System/Library/PrivateFrameworks/MediaRemote.framework/MediaRemote", RTLD_LAZY);
        if (framework != nullptr) {
            Claim = (SetCanBeNowPlayingApplication)dlsym(framework, "MRMediaRemoteSetCanBeNowPlayingApplication");
        }
        if (Claim == nullptr) {
            Claim = (SetCanBeNowPlayingApplication)1;
        }
    }
    if (Claim == (SetCanBeNowPlayingApplication)1) {
        return (false);
    }
    Claim(active ? 1 : 0);
    return (true);
}

} // namespace

void Media_Keys_Claim(bool active, char const* title)
{
    if (!Installed) {
        MPRemoteCommandCenter* center = [MPRemoteCommandCenter sharedCommandCenter];
        [center.nextTrackCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent*) {
            dispatch_async(dispatch_get_main_queue(), ^{
                Media_Keys_Step(1);
            });
            return (MPRemoteCommandHandlerStatusSuccess);
        }];
        [center.previousTrackCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent*) {
            dispatch_async(dispatch_get_main_queue(), ^{
                Media_Keys_Step(-1);
            });
            return (MPRemoteCommandHandlerStatusSuccess);
        }];
        Installed = true;
    }

    char const* name = (title != nullptr) ? title : "";
    if (active == Enabled && std::strncmp(Title, name, sizeof(Title)) == 0) {
        return;
    }
    Enabled = active;
    std::strncpy(Title, name, sizeof(Title) - 1);
    Title[sizeof(Title) - 1] = '\0';

    Claimed = Claim_Now_Playing(active) && active;

    MPRemoteCommandCenter* center = [MPRemoteCommandCenter sharedCommandCenter];
    center.nextTrackCommand.enabled = active;
    center.previousTrackCommand.enabled = active;

    MPNowPlayingInfoCenter* info = [MPNowPlayingInfoCenter defaultCenter];
    if (!active) {
        info.playbackState = MPNowPlayingPlaybackStateStopped;
        info.nowPlayingInfo = nil;
        return;
    }

    NSString* label = (Title[0] != '\0') ? [NSString stringWithUTF8String:Title] : @"Komkommer";
    info.playbackState = MPNowPlayingPlaybackStatePlaying;
    info.nowPlayingInfo = @{
        MPMediaItemPropertyTitle : label,
        MPNowPlayingInfoPropertyMediaType : @(MPNowPlayingInfoMediaTypeAudio),
        MPNowPlayingInfoPropertyPlaybackRate : @1.0,
    };
}

bool Media_Keys_Claimed(void)
{
    return (Claimed);
}
