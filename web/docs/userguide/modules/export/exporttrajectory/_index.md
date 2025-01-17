---
title: ExportTrajectory (Module)
linkTitle: ExportTrajectory
description: Export configuration coordinates to a trajectory file
---

## Overview

The `ExportTrajectory` module writes the coordinate data from a target configuration to a named trajectory file, appending frames on successive calls. As such it can be used to generate a trajectory of coordinates for analysis by external codes.

## Options

### Targets

|Keyword|Arguments|Default|Description|
|:------|:--:|:-----:|-----------|
|`Configuration`|`Configuration`|--|{{< required-label >}}Target configuration on which to operate.|

### File

|Keyword|Arguments|Default|Description|
|:------|:--:|:-----:|-----------|
|`Format`|[`TrajectoryFileAndFormat`]({{< ref "trajectoryformat" >}})|--|Format and filename of the target trajectory to write to disk.|
