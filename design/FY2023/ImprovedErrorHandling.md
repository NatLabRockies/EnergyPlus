# Modern Error Reporting

One feature request pretty much summarizes the issue and solution for our error reporting:

> "The current solution of writing out the error file to a text file without establishing a standard procedure makes it difficult for third-party vendors to translate the content.
  We would prefer to start establishing a standard template for warnings/error records with unique IDs being written to the JSON files. "

OK, so the big picture is we want an error file in JSON format with some form of ID associated with the error messages.
A nice side benefit of this would be that we could generate a bit of documentation that cross references the ID.
The documentation could be built up over time with additions like typical causes, tips and tricks for fixing or working around, etc.

## Detailed task list

To meet this need, I think development would follow something like this:

- Analyze existing structure, figuring out a feasible plan for refactoring error message calls
- Create a new error manager in EnergyPlus that tracks errors in a nice structure
- Refactor calls to error message routines to use a vector of strings rather than individual calls to *Continue*
- Refactor the error message routines to not just emit to the err file but also add them to the new error manager
  - (at this point I should be able to get no diffs)
- Try to find "nearly" identical error messages and bring them together
  - I would expect this to cause valid diffs in the err file as minor wording changes are collected together
- Create a set of ID categories, and modify the error emission to report the error ID
- Use the error manager to generate the JSON error file
- Create an auto-generated document for the different error message IDs

## Error File Contents

I would start creating the error file with something like this, but am totally open to feedback, and it also may naturally change as development commences:

```json
{
  "summary": {
    "outcome": "fatal",
    "num_severe": 2,
    "num_warnings": 1,
    "runtime": 210.2,
    "timestamp": "2023-04-24T13:05:30Z"
  },
  "fatal": {
    "id": "F1000",
    "summary": "EnergyPlus failed because of a Meta-reason",
  },
  "severe": [
    {
      "id": "S1021",
      "synopsis": "Flow controller did not converge",
      "extra_info": "Air loop 'AirLoop' diverged with oscillating flow rate.  Flow controller 'ControlStrategy' is set to 'InverseMobiusStrip', which only exists in hyper dimensional systems.  Choose a more appropriate option."
    },
    {
      "id": "S3001",
      "synopsis": "Encountered meta-building in the input file",
      "extra_info": "Building 'MetaBuilding' was declared with meta-latitude '\\342\\200\\234' and meta-longitude '\\742\\234\\876', which are not supported in this version of EnergyPlus"
    }
  ],
  "warnings": [
    {
      "id": "W4040",
      "synopsis": "Timestep is set to 3600 seconds, which is incompatible with some cool short time step model, suggest setting it to 3599 or less.",
      "extra_info": ""
    }
  ],
  "information": [
    "Testing Individual Branch Integrity",
    "All Branches passed integrity testing",
    "Testing Individual Supply Air Path Integrity",
    "All Supply Air Paths passed integrity testing",
    "Testing Individual Return Air Path Integrity",
    "All Return Air Paths passed integrity testing",
    "No node connection errors were found.",
    "Beginning Simulation",
    "EnergyPlus Completed Successfully"
  ]
}
```

We'll need to keep in mind how new errors can be introduced into the category system to future proof it.
Many tools do something like: A001, A002, B001, B002, so I would probably start by doing something similar.

Things I still need to solve:
- Should I include timestamp info for every single warning?
- Add structure for recurring
- Add extra flag for whether warmup, sizing, or kickoff were true

## Current questions for dev team

- What should we name this JSON based error file?
- Do we just keep a list of message string templates in a single dedicated file, and use these in calls to errors?
- Anyone have strong feelings on the ID or JSON form?
