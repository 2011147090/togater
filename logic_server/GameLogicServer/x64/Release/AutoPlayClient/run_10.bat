start AutoPlayClient.exe 1 true

for /L %%i in (2, 1, 10) do (
    start /b AutoPlayClient.exe %%i false
)