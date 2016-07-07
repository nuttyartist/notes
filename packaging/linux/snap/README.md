This project builds [Notes](http://www.get-notes.com/) and packages it as a
snap for multiple Linux distros.

![notes](https://cloud.githubusercontent.com/assets/16375940/14313739/ea9fc8fa-fbfb-11e5-95bb-fb10c59770a8.png)

To build it on an Ubuntu 16.04 machine, simply go to the `notes` directory and type:

    cd src
    qmake
    make snap
    
**Developers**: to install the locally built snap (i.e. sideload),

    sudo snap install notes_0.0.8~snap3.gita80fd1c_amd64.snap

Note: replacing `0.0.8~snap3.gita80fd1c` by the revision of the resulting app you've built. The version is defined on the `snapcraft.yaml` file.

**Users**: to install the app from the Ubuntu Store:

    sudo snap install notes

[Learn more at the Snapcraft site](http://snapcraft.io)

## Snapcraft features

This project uses the following Snapcraft features
- The qmake plugin
- A [wiki-based](https://wiki.ubuntu.com/snapcraft/parts) part (`desktop/qt`) needed to set up the environment before starting Qt apps.

## App functionality

The application works fine so far. Note that due to confinement data is being written to `/home/$USER/snap/notes`.

### Warnings

    XmbTextListToTextProperty result code -2
    XmbTextListToTextProperty result code -2
    XmbTextListToTextProperty result code -2

It's still unknown where the warnings come from, but seems to be harmless.

### AppArmor denials

    Apr 21 13:30:06 host kernel: [118072.276749] audit: type=1400 audit(1461238206.897:1500): apparmor="DENIED" operation="chmod" profile="snap.notes-dpm.notes" name="/var/cache/fontconfig/" pid=15243 comm="Notes" requested_mask="w" denied_mask="w" fsuid=1000 ouid=0

Denial trying to write to `/var/cache/fontconfig`, which seems to be harmless, but quite common amongst snaps.
