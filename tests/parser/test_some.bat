for %%f IN (1,2,3,4,5) do (
	"..\..\Debug\pascal compiler.exe" 0%%f.in -p > 0%%f.in.out
)
pause 
