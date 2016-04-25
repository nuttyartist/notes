# -*- Mode:Python; indent-tabs-mode:nil; tab-width:4 -*-
#
# Copyright (C) 2016 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author(s): David Planella <david.planella@ubuntu.com>

import os

import snapcraft
from snapcraft.plugins import make
import sysconfig

class QmakePlugin(make.MakePlugin):

    def __init__(self, name, options, project):
        super().__init__(name, options, project)

    def build(self):
        """Build the source code as retrieved from the pull phase.

        Being based on qmake, the project needs the `qmake` command to be run
        before doing the build. The `qmake` command generates a Makefile that
        can then be processed with standard `make`.
        """

        # Skip over parent class to copy the source dir
        # to the build dir
        snapcraft.BasePlugin.build(self)

        # Define commands to be run
        qmake_command = ['qmake', 'PREFIX={}'.format(self.installdir),
                         os.path.join(self.builddir, 'Notes.pro'),
                         '-o', os.path.join(self.builddir, 'Makefile')]
        make_command = ['make']

        # Run qmake to generate a Makefile
        self.run(qmake_command)

        # TODO: this does not seem to work - it does not find the Makefile
        #super().build()

        # Run make to build the sources
        self.run(make_command +
                 ['-j{}'.format(self.project.parallel_build_count)])

        # Run make to install the binaries
        self.run(make_command + ['install'])
