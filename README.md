# Generative OpenFrameworks Artworks

This repository contains multiple generative art projects built with openFrameworks. Each project lives in its own subfolder.

## Structure

- `template/` — openFrameworks project (source in `src/`, build outputs in `bin/`)
- `.vscode/` — VS Code tasks for building/running with MSYS2/MinGW64
- `generative-of.code-workspace` — VS Code workspace file
- `README.md` — this file

Additional project folders can be added alongside `template/`.

## Requirements

- openFrameworks (installed separately). This repo should sit in the openFrameworks `apps/` folder.
- Windows + MSYS2/MinGW64 toolchain
  - Use `C:\msys64\mingw64.exe` as your terminal and build environment for this repo.
  
[see OpenFrameworks msys2 setup](https://openframeworks.cc/setup/msys2/)

## Adding a New Project

Project creation is done by copy/pasting the `template/` project folder and renaming it. You can also use the OpenFrameworks project generator tool, but I added some debug/saving tool built into the template.

1. Duplicate the `template/` folder and rename the copy to your new project name (avoid spaces).
2. Edit `src/` to implement your artwork.
3. Update `addons.make` if you need to add or remove addons.
4. Use the same VS Code tasks to build and run; select the subproject when prompted (defaults to `template`).
5. To have your new project appear in the VS Code task picker, add its folder name to `.vscode/tasks.json` under `inputs -> subproject -> options`.

Notes:

- The openFrameworks make system infers the app name from the folder name. Renaming the folder is typically sufficient.
- Ensure you’re using the MSYS2/MinGW64 shell when running builds.

## Usage

- Open this repository in VS Code.
- Use the Tasks panel to build and run via MSYS2/MinGW64.
- Artifacts are created under each project’s `bin/` folder.

## License

MIT License. See the `LICENSE` file for details.
