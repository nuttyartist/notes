This project builds [Notes](http://www.get-notes.com/) and packages it as a
snap for Ubuntu.

![notes](https://cloud.githubusercontent.com/assets/16375940/14313739/ea9fc8fa-fbfb-11e5-95bb-fb10c59770a8.png)

To build, simply go to the `notes` directory and type:

    snapcraft

## Snapcraft features

This project uses the following Snapcraft features
- A custom Snapcraft plugin, derived from the `make` plugin. This is necessary to call the qmake step before doing the build
- A [wiki-based](https://wiki.ubuntu.com/Snappy/Parts) part (`qt5conf`) needed to set up Qt apps to select their Qt version before starting
- A [wrapper](https://github.com/dplanella/snappy-playpen/blob/master/notes/notes.wrapper) that sets up environment variables before calling the binary
- The `copy` plugin to install the wrapper

## App functionality

The application works fine so far, apart from some icons not being shown. This seems to be the case for the upstream version shipped in the Debian package, so it's likely not caused by the snap packaging or confinement.

### Warnings

    Qt: Session management error: None of the authentication protocols specified are supported
    XmbTextListToTextProperty result code -2
    XmbTextListToTextProperty result code -2
    XmbTextListToTextProperty result code -2

The first warning might need setting an environment variable on the wrapper. The subsequent warnings are most likely due to upstream.

### AppArmor denials

    Apr 21 13:30:06 host kernel: [118072.276749] audit: type=1400 audit(1461238206.897:1500): apparmor="DENIED" operation="chmod" profile="snap.notes-dpm.notes" name="/var/cache/fontconfig/" pid=15243 comm="Notes" requested_mask="w" denied_mask="w" fsuid=1000 ouid=0

Denial trying to write to `/var/cache/fontconfig`, which seems to be quite common amongst snaps.
