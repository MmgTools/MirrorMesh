# MirrorMesh - 3D Mesh mirroring

MirrorMesh is an open source software to perform mesh mirroring (planar symmetry).
It uses the [Mmg](http://mmgtools.org) software for mesh management.

## Get and compile the MirrorMesh project
### Needed tools
To get and build MirrorMesh, you will need:
 * **Git**: to download the code you will have to use a git manager. You can install a git manager from the link below but there are many other git clients that you can use:
    * [Official Git client](https://git-scm.com/download) (command line program)
    * [GitKraken](https://www.gitkraken.com/)
    * [SourceTree](https://www.sourcetreeapp.com/)  

    Note that if you uses Microsoft Visual Studio (Windows OS), you can simply activate the Git Module of the application.

  * **CMake** : MirrorMesh uses the CMake building system that can be downloaded on the
    following web page:
    [https://cmake.org/download/](https://cmake.org/download/). On Windows OS,
    once CMake is installed, please <span style="color:red"> do not forget to
    mark the option: 
    ```
    "Add CMake to the system PATH for all users"
    ```
    </span>  

### MirrorMesh download and compilation
#### Unix-like OS (Linux, MacOS...)

  1. Get the repository:  
```Shell
      git clone https://github.com/MmgTools/MirrorMesh.git
```

  The project sources are available under the **_src/_** directory.

  2. By default MirrorMesh download and install automatically Mmg (as a CMake external project):
```Shell
      cd MirrorMesh  
      mkdir build  
      cd build  
      cmake ..  
      make  
      make install
```
  If the `make install` command fail, try to run the `sudo make install` command.
  If you don't have root access, please refers to the [Installation section](https://github.com/MmgTools/Mmg/wiki/Setup-guide#iii-installation) of the [setup guide](https://github.com/MmgTools/Mmg/wiki/Setup-guide#setup-guide) of Mmg and follow the same steps.

  If you don't have internet access and/or want to use your own installation of
  Mmg, you can disable the automatic download of Mmg setting the
  `DOWNLOAD_MMG` CMake variable to `OFF`. In this case,
  you have to help CMake to find Mmg by specifying the source
  directory of Mmg in the `MMG_DIR` variable and the build directory of Mmg in
  the `MMG_BUILDDIR` variable.
  
  Example:
  ```Shell
      cd MirrorMesh  
      mkdir build  
      cd build  
      cmake -DDOWNLOAD_MMG=OFF -DMMG_DIR=~/mmg -DMMG_BUILDDIR=~/mmg/build ..  
      make  
      make install
```

  The **MirrorMesh** application is available under the `mirrormesh_O3` command.

Note that if you use some specific options and want to set it easily, you can use a shell script to execute the previous commands. An example is provided in the Mmg wiki [here](https://github.com/MmgTools/mmg/wiki/Configure-script-for-CMake-(UNIX-like-OS)).

## Documentation
MirrorMesh replicates a mesh by mirroring along each direction. The
number of mirrors along each axis can be provided using the `-nx <nx>
-ny <ny> -nz <nz>` command line arguments. Thus, the following command
line apply 1 mirror along the x-axis, 2 mirrors along the y-axis and 5
along the z-axis to the `input.mesh` mesh and saves the result in the `output.mesh` mesh:
```
mirrormesh_o3 -nx 1 -ny 2 -nz 5 input.mesh output.mesh
```


### About the team
MirrorMesh's current developers and maintainers are:
  * [Algiane Froehly](mailto:algiane.froehly@inria.fr).

## License and copyright
Code is under the [terms of the GNU Lesser General Public License](https://raw.githubusercontent.com/MmgTools/MirrorMesh/master/LICENSE).

Copyright © Inria 2021 -
