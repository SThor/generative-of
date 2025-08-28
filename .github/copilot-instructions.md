<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

Verify that all previous steps have been completed successfully and you have marked the step as completed.
Develop a plan to modify codebase according to user requirements.
Apply modifications using appropriate tools and user-provided references.

Build/Run
- **NEVER attempt to build or run tasks yourself!** Always leave building/running to the user once you've completed all requested code changes.
- Only make code edits, fixes, and file modifications - the user handles all compilation and execution.
- If the user reports build errors, analyze and fix the code issues, but don't attempt to build again.
- Suggest "Clean ALL" and rebuild if build errors reference generated objects, but let the user execute it.

Checklist
- Confirm this is an openFrameworks workspace with multiple app subfolders (e.g., template/).
- Use MSYS2/MinGW64 for builds; prefer the existing VS Code tasks.
- For a new app: duplicate the existing template app folder (e.g., template/), rename it.
- Keep changes scoped to the selected subproject; never move or rename the openFrameworks core libs.
- When editing, avoid large refactors unless requested. Prefer minimal, focused patches.
- **IMPORTANT**: For shader projects, always ensure `settings.setGLVersion(3, 2);` is set in main.cpp for proper GLSL #version 150 support.

Documentation
- Keep README structure in sync with actual folders.
- Use MIT for license notices.

Communication
- Be concise. Offer concrete edits. Run tasks when verification is needed.

