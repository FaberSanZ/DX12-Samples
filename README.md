##               DirectX 12 Ultimate® Samples

  
</h1>


  ##              



## Overview
This is designed for the DX12 learning process, I mainly use it to experiment with graphical or computing techniques and should not be used as a cerium product as it may have memory leaks and faulty or poorly optimized implementations.




## Examples


Example | Details
---------|--------
<img src="Screenshots/ClearScreen.png" width=380> | [Clear Screen](Src/ClearScreen)<br> This sample shows how to initialize DirectX 12, select a GPU (adapter), create a device, a command queue, a swap chain, a render target view (RTV) descriptor heap, and a command list to clear the screen to a solid color. All rendering commands are recorded into the command list, then executed through the command queue. If you pass nullptr when choosing the adapter, the default GPU (usually GPU 0) is selected.
