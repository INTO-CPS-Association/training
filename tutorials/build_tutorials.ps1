$tutorial_files = Get-ChildItem -Path . -Filter "*.tex" -Recurse

foreach ($file in $tutorial_files) {
    Push-Location $file.Directory
        echo "Building $file"
        $filename = $file.Basename
        & latexmk -C
        & latexmk.exe -pdf -silent "$file"
    Pop-Location
}