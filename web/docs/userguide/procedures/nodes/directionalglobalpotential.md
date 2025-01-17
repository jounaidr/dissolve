---
title: DirectionalGlobalPotential (Node)
linkTitle: DirectionalGlobalPotential
description: Create a directional global potential affecting all atoms
---

{{< htable >}}
| | |
|--------|----------|
|Context|Generation|
|Name Required?|No|
|Branches|--|
{{< /htable >}}

## Overview

The `DirectionalGlobalPotential` node allows an additional, global potential to be defined in a configuration, acting on all atoms and in a specified direction.

## Options

### Definition

|Keyword|Arguments|Default|Description|
|:------|:--:|:-----:|-----------|
|`Potential`|[`DirectionalPotential`]({{< ref directionalpotential >}})|--|Functional form and associated parameters for the potential.|
