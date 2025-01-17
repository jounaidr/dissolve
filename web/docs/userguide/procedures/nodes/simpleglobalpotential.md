---
title: SimpleGlobalPotential (Node)
linkTitle: SimpleGlobalPotential
description: Create a simple, global potential affecting all atoms
---

{{< htable >}}
| | |
|--------|----------|
|Context|Generation|
|Name Required?|No|
|Branches|--|
{{< /htable >}}

## Overview

The `SimpleGlobalPotential` node allows an additional, global potential to be defined in a configuration, acting on all atoms.

## Options

### Definition

|Keyword|Arguments|Default|Description|
|:------|:--:|:-----:|-----------|
|`Potential`|[`SimplePotential`]({{< ref simplepotential >}})|--|Functional form and associated parameters for the potential.|
