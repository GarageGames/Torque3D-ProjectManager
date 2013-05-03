Torque3D-ProjectManager
=======================

MIT Licensed version of the [Torque 3D](http://www.garagegames.com/products/torque-3d) Project Manager from [GarageGames](http://www.garagegames.com)

More Information
----------------
The Torque 3D Project Manager may be used to create new projects based on existing Torque 3D templates.  It is based on the open source Qt GUI framework by Digia.

Building Qt for Windows
-----------------------
Before you may compile the Project Manager you will need to build Qt 4.8.2.  Please follow the directions at [Installing Qt for Windows](http://qt-project.org/doc/qt-4.8/install-win.html) up to Step 4.

Building Project Manager for Windows
------------------------
1. Open a Visual Studio Command Prompt (2010).
2. Go to the `Torque3D-ProjectManager/buildFiles/VisualStudio` directory.
3. Type `qmake` and press return.  This will create a Visual Studio project file.
4. Start VS2010 and load the Project Manager project.
5. Build a release version.  You will be asked to save the solution file before the compilation will begin.  Save with the default name.
6. `Project Manager.exe` will be created in `Torque3D-ProjectManager/bin/win32`.

Pre-Compiled Version
--------------------
A pre-compiled version of the Project Manager along with the necessary support files may be found in the here: [Downloads page of Torque 3D repo](https://github.com/GarageGames/Torque3D/wiki/Project-Manager-Archive)

Setting Up Project Manager under Windows
----------------------------------------
1. Remove the existing `Torque 3D Toolbox.exe` file and the four Qt .dll files from your Torque 3D repo.
2. Copy `Project Manager.exe` and `projects.xml` from `bin/win32` into your Torque 3D repo.
3. Copy `QtCore4.dll`, `QtGui4.dll`, `QtNetwork4.dll` and `QtXml4.dll` from where they were compiled (usually in the Qt's `lib` directory) into the Torque 3D repo.

Using Project Manager
---------------------
1. Double click `Project Manager.exe` to start it up.
2. Click on the `New Project` button to create a project from an existing Torque 3D template.  Click on the `Choose Modules` button to change which modules will be included with your project.
3. With a project selected, click on the `Open Folder` button to open a new window at the project's location.
4. With a project selected, click on the `Modules` button to modify which modules an existing project will compile with.
5. With a project selected, click on the `Regenerate` button to regenerate the project's C++ files.  This is the same as launching the `generateProjects.bat` file.

License
-------

Copyright (c) 2012 GarageGames, LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
