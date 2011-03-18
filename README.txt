=====================================================
Codeinfo - a code information browser plugin for Kate
=====================================================


Description
===========

Codeinfo provides an *in-Kate* way of navigating through source code information such as the output of a static code analysis tool, a compiler or a debugger. 

Codeinfo runs the selected external tool, parses the result and displays it within Kate, allowing to go to the corresponding file, line and column. You must make sure that the desired tools are installed and available in the path, as Codeinfo does not provide them.

Each tool consists of three parts: the name which is used to display, the command that is used to execute it and a regular expression that is used to parse the results.

Currently, the following tools are predefined:
- pep8

You can add your own tools through the configuration window. Parsing is done using regular expressions in a line by line fashion. The syntax used to interpret the meaning of the pattern is RegExp2 as defined by `Qt 4.7 <http://doc.qt.nokia.com/4.7/qregexp.html>`_. Named captures had beed added to the syntax for greater flexiblity. For example `(P<message>.*)` means that the whole content of the line will be used in the field ``message``


Installing
==========

You will need to have cmake installed and the development packages of kate, kde and qt.

First, download the code or clone the git repo:
    
    git clone git@github.com:hgrecco/KateCodeinfo.git  
   
Then build and install the plugin:
    
    cd KateCodeinfo
    mkdir build
    cd build
    cmake ../src
    make 
    sudo make install
    
In some distros, the output files will be installed in the wrong folder and Kate cannot find them.

For example:
- /usr/local/lib/kde4/katecodeinfoplugin.so instead of /usr/lib/kde4/katecodeinfoplugin.so
- /usr/local/share/kde4/services/katecodeinfoplugin.desktop instead of /usr/share/kde4/services/katecodeinfoplugin.desktop

To overcome this problem you can use (for this particular example):

    cmake ../src -DCMAKE_INSTALL_PREFIX=/usr

or just copy the files to the right folder. If you do not know which is the right folder, just look in your file system for the following files katekonsoleplugin.so and katekonsoleplugin.desktop


Contribute
==========

If you'd like to hack on KateCodeinfo, you can start by forking my repo on GitHub:

   http://github.com/hgrecco/KateCodeinfo

hack, test and then send me a pull request.

I keep two branches on GitHub.
- master: appropriate for users. It is always working.
- develop: appropriate for developers. Might not be in a fully usable state.

Additionally, other branches might appear as testing ground for new features.


Authors
=======

- Hernan E. Grecco <hernan.grecco@gmail.com>

