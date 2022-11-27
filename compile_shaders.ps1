$path = ".\assets\shaders"
Get-ChildItem "$path\*" -Include "*.frag", "*.vert" |
Foreach-Object {
    Write-Output "Compiling $_..."
    glslc.exe $_ -o "$_.spv"
}

