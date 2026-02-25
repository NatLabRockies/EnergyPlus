# Data-Driven GitHub Stale Issue Management for EnergyPlus

**Brian L Ball, NLR**
- Original Date: February 2026
- Final Date: TBD

------------------------------------------------------------------------

## Justification

The EnergyPlus GitHub repository currently contains a significant
backlog of open issues that have not been updated in more than two years.
Based on internal analysis:

-   581 open issues are unassigned and inactive for \> 2 year
-   141 open issues are assigned but inactive for \> 2 year
-   230 open issues labeled `NewFeatureRequest` are inactive for \> 2
    year

This volume of stale issues:

-   Obscures active work
-   Makes triage more difficult
-   Discourages contributors
-   Increases cognitive load for maintainers
-   Distorts project health metrics

Currently, stale issue management is manual and inconsistent.

This NFP proposes implementing a data-driven GitHub Actions workflow
using `actions/stale` to:

1.  Systematically identify inactive issues.
2.  Notify reporters before closure.
3.  Close inactive issues after a grace period.
4.  Route New Feature Requests (NFRs) to the formal NFP process via a
    documented URL.
5.  Preserve assigned issues and high-priority categories.

------------------------------------------------------------------------

## Conference Call Conclusions

To be completed following review.

------------------------------------------------------------------------

## Overview

### Introduction

This proposal introduces automated lifecycle management for stale GitHub
issues in the EnergyPlus repository using `.github/workflows/stale.yml`.

The implementation will be data-driven and configurable to minimize
disruption.

### Proposed Feature Summary

Two automated workflows are proposed:

1.  **General Stale Policy**
    -   Target: Unassigned issues inactive \> 365 days
    -   Action: Label as `Stale`
    -   Auto comment something like: `automatically labeled as Stale, will close in 30days unless updated`
    -   After 30 days without activity: Close automatically
2.  **New Feature Requests (NFRs)**
    -   Target: Issues labeled `NewFeatureRequest` inactive \> 365 days
    -   Action:
        -   Post comment directing reporter to official NFP page (`URL to be inserted`)
        -   Close issue
3.  **General Assigned Policy**
    -   Target: Assigned issues inactive \> 30 days
    -   Action: Label as `Stale`
    -   Auto comment something like: `automatically labeled as Stale, will unassign in 30days unless updated`
    -   After 30 days without activity: unassign developer

### Policy Rules

| Category              | Stale After | Close After     | Notes                      |
|-----------------------|-------------|-----------------|----------------------------|
| Unassigned, non-NFR   | 365 days    | 30 days         | Primary cleanup target     |
| NewFeatureRequest     | 365 days    | Immediate close | Direct to NFP URL          |
| Assigned issues       | 30 days     | Exempt          | Unassign / not close       |
| `??`                  | Exempt      | Exempt          | Protected categories       |

## New Feature Request Routing

NewFeatureRequest issues inactive \> 365 days will:

1.  Receive a comment directing users to: (`URL to be inserted`)
2.  Be closed.

### NFR Closure Messaging

New Feature Requests will receive a comment such as:

> This issue is being closed due to inactivity.
>
> New features for EnergyPlus require submission through the formal New
> Feature Proposal (NFP) process.
>
> Please review and submit your proposal here:
>
> **`<INSERT NFP URL HERE>`{=html}**
>
> If an NFP is submitted and approved, a new implementation issue will
> be created.

------------------------------------------------------------------------

## Workflow Implementation

-   Use `actions/stale@v10`
-   Scheduled daily via cron
-   Workflow dispatch enabled for manual testing

``` yaml
name: Manage stale issues

on:
  schedule:
    - cron: "30 2 * * *"
  workflow_dispatch:

permissions:
  issues: write

jobs:
  stale-unassigned-non-nfr:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/stale@v10
        with:
          exempt-all-assignees: true
          exempt-issue-labels: "security,pinned"

          stale-issue-label: "Stale"
          close-issue-reason: "not_planned"

          days-before-issue-stale: 365
          days-before-issue-close: 30

          stale-issue-message: >
            This issue has been marked as stale due to inactivity.
            If this is still relevant, please comment with updated details.

          close-issue-message: >
            Closing due to prolonged inactivity. Reopen with updated details if needed.

          remove-stale-when-updated: true
          operations-per-run: 200

  stale-nfr:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/stale@v10
        with:
          any-of-labels: "NewFeatureRequest"

          days-before-issue-stale: 365
          days-before-issue-close: 0

          close-issue-reason: "not_planned"

          stale-issue-message: >
            New features require submission through the formal NFP process.

          close-issue-message: >
            This issue is being closed due to inactivity.
            Please submit a formal New Feature Proposal here:
            https://energyplus.net/new-feature-proposal
```

------------------------------------------------------------------------

### Rollout Plan

Phase 1 -- Debug-only dry run (no changes applied)
Phase 2 -- Label-only mode (no auto-close)
Phase 3 -- Enable auto-close for non-NFR issues
Phase 4 -- Enable NFR closure routing

Metrics will be collected during rollout.

------------------------------------------------------------------------

## Testing/Validation Data Sources

Validation metrics:

-   Count of stale issues labeled
-   Count of issues closed
-   Rescue rate (issues updated after stale label)
-   Reduction in backlog size
-   Time-to-triage improvements

Testing approach:

-   Enable `debug-only: true`
-   Run on fork or test branch
-   Confirm exemption rules behave as expected
-   Validate label application before closure

------------------------------------------------------------------------

## IORef/Draft

Not applicable.

------------------------------------------------------------------------

## Proposed Report Variables

Not applicable.

------------------------------------------------------------------------

## Proposed Additions to Meters

Not applicable.

------------------------------------------------------------------------

## Engineering Reference Draft

Not applicable.

------------------------------------------------------------------------

## Example File

`.github/workflows/stale.yml`

------------------------------------------------------------------------

## Transition Changes

No simulation transition impacts.
Repository workflow change only.

------------------------------------------------------------------------

## Other Documents

-   EP_issues.pptx (backlog data analysis)
-   GitHub documentation for `actions/stale`
-   Official EnergyPlus NFP process documentation (URL to be inserted)
