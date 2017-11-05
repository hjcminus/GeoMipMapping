@echo on
for %%f in (*.vert) do ( fxc /T vs_5_0 /Fo binary\%%~nf.vert.bin %%~nf.vert )
for %%f in (*.frag) do ( fxc /T ps_5_0 /Fo binary\%%~nf.frag.bin %%~nf.frag )
pause