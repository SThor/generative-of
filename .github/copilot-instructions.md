<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

Verify that all previous steps have been completed successfully and you have marked the step as completed.
Develop a plan to modify codebase according to user requirements.
Apply modifications using appropriate tools and user-provided references.

Repo-specific Copilot instructions:

Checklist
- Confirm this is an openFrameworks workspace with multiple app subfolders (e.g., template/).
- Use MSYS2/MinGW64 for builds; prefer the existing VS Code tasks.
- For a new app: duplicate the existing template app folder (e.g., template/), rename it.
- Keep changes scoped to the selected subproject; never move or rename the openFrameworks core libs.
- When editing, avoid large refactors unless requested. Prefer minimal, focused patches.

Build/Run
- You have a bit of trouble running the build & run tasks, so simply leave it to the user once you've done all the requested changes.
- If the task prompts for a subproject, choose the intended folder.
- If build errors reference generated objects, clean via “Clean ALL” and rebuild.

Documentation
- Keep README structure in sync with actual folders.
- Use MIT for license notices.

Communication
- Be concise. Offer concrete edits. Run tasks when verification is needed.

