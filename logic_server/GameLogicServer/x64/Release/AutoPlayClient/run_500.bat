start AutoPlayClient.exe 1 true

for /L %%i in (2, 1, 500) do (
    start /b AutoPlayClient.exe %%i false
)