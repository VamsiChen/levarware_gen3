# levaware_gen3
Refactor Levaware product firmware for improved power consumption and improved design. 

This code is:
    - Compiled with Nordic SDK version 2.0.0
    - Tool chain version 2.0.0

Problems have been found when using the latest Nordic VS Code extensions with older toolchains (ex: the debugger would crash). 
The Nordic VS Code extensions used for development are: 
    •	nordic-semiconductor.nrf-connect – 2023.2.56
    •	nordic-semiconductor.nrf-devicetree – 2023.10.22
    •	nordic-semiconductor.nrf-terminal – 2023.10.17
    •	ms-vscode.cpptools – 1.17.5
    •	nordic-semiconductor.nrf-kconfig – 2023.10.27

    Note: if you find you are having issues with the debugger crashing on this early version of the tool chain, 
    I suggest setting your extensions to the above versions (that's what worked for me). 

    I assume this will not be an issue when using newer versions of the SDK / Toolchain. 
    