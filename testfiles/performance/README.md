This will be the location for holding files we choose for inclusion in performance-related studies.  Some things to consider:
 - Versioning: Do we only store the files in coordination with the develop branch, so they may change at each commit?  Or do we store separate folders with individual versions?
 - CI: Do we need to add these to the CI testing framework?  Eventually probably so, but for the time being, I think it will just add time to the testing for each commit.
 - File selection: We should be careful to keep this set of files under control, with each file specifically exercises an aspect of EnergyPlus that is assumed to be performance-centric (one file with large I/O, no computation; another opposite; etc.)

For now, feel free to just drop files in, and we'll adapt this as we figure out the best plan.
