Pull request overview
---------------------

- Fixes #ISSUENUMBERHERE <!-- IF THIS IS A DEFECT -->

<!-- NOTE: ENHANCEMENTS MUST FOLLOW A SUBMISSION PROCESS INCLUDING A FEATURE PROPOSAL AND DESIGN DOCUMENT PRIOR TO SUBMITTING CODE -->

### Description of the purpose of this PR

<!-- DESCRIBE PURPOSE OF THIS PULL REQUEST -->

### Pull Request Author

<!-- Add to this list or remove from it as applicable.  This is a simple templated set of guidelines. -->

 - [ ] Title of PR should be user-synopsis style (clearly understandable in a standalone changelog context)
 - [ ] Label the PR with at least one of: Defect, Refactoring, NewFeature, Performance, and/or DoNoPublish
 - [ ] Pull requests that impact EnergyPlus code must also include unit tests to cover enhancement or defect repair
 - [ ] Author should provide a "walkthrough" of relevant code changes using a GitHub code review comment process
 - [ ] If any diffs are expected, author must demonstrate they are justified using plots and descriptions
 - [ ] If changes fix a defect, the fix should be demonstrated in plots and descriptions
 - [ ] If any defect files are updated to a more recent version, upload new versions here or on DevSupport
 - [ ] If IDD requires transition, transition source, rules, ExpandObjects, and IDFs must be updated, and add IDDChange label
 - [ ] If structural output changes, add to output rules file and add OutputChange label
 - [ ] If adding/removing any LaTeX docs or figures, update that document's CMakeLists file dependencies
 - [ ] If adding/removing any output files (e.g., eplustbl.*)
   - [ ] Update ..\scripts\Epl-run.bat
   - [ ] Update ..\scripts\RunEPlus.bat
   - [ ] Update ..\src\EPLaunch\ MainModule.bas, epl-ui.frm, and epl.vbp (VersionComments)
   - [ ] Update ..\.github\workflows\energyplus.py

### Reviewer

<!-- This will not be exhaustively relevant to every PR. -->

 - [ ] Perform a Code Review on GitHub
 - [ ] If branch is behind develop, merge develop and build locally to check for side effects of the merge
 - [ ] If defect, verify by running develop branch and reproducing defect, then running PR and reproducing fix
 - [ ] If feature, test running new feature, try creative ways to break it
 - [ ] CI status: all green or justified
 - [ ] Check that performance is not impacted (CI Linux results include performance check)
 - [ ] Run Unit Test(s) locally
 - [ ] Check any new function arguments for performance impacts
 - [ ] Verify IDF naming conventions and styles, memos and notes and defaults
 - [ ] If new idf included, locally check the err file and other outputs
