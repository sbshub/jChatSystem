# jChatSystem

jChatSystem is a TCP network based chat system, using clients and a server. It is a very basic way to communicate using a command line as the interface. It will feature a room-based system and will require client authentication.

## Features

The following is a list of planned and current features:
* User identification
* Direct messaging
* Channel management
  * Ban user
  * Kick user
  * Op user *
  * Deop user *
  * Unban user *
  * `* Planned`
* Channel messaging

## Installation/Usage

1. Download, or clone repository using `git clone https://github.com/Imposter/jChatSystem.git`
2. Set up the project using `premake5_* [gmake|vs2013|vs2015]`. Depending on your platform you can use `premake5_linux`, `premake5_osx`, `premake5_windows.exe`
3. Once you've created the project, you can either open `jchat.sln` or build using:
  * Configurations:
    * debug_win32 *
    * debug_win64
    * debug_unix32 *
    * debug_unix64
    * release_win32 *
    * release_win64
    * release_unix32 *
    * release_unix64
    * `* GNU compilers may not work`
  * `make config=debug_unix32 all`
4. You can launch the program in the corresponding platform and configuration in the `build/` directory

## Contributing

#### Users with access to this repository
1. Clone the repository
2. Adding new directories or files to be monitored by git: `git add <path>`
3. Update commit with relevant changes `git stage .` (Updates commit with all changes)
4. Commit your changes: `git commit -m "Your message"`
5. Push your commit: `git push -u origin charlie`
6. Enter your credentials when prompted.

#### Users without access to this repository
1. Fork it!
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am "Added some new features"`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request.

`NOTE: When you submit a pull request, we'll evaluate your code and send you feedback on it!`

## License

jChatSystem - Another Chat System

Copyright (C) 2016 Eyaz Rehman & Shubham Patel. All Rights Reserved.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA 02110-1301, USA.
