---
title: Process3D (Node)
linkTitle: Process3D
description: Process a 3D histogram
---

{{< htable >}}
| | |
|-|-|
|Context|Analysis|
|Name Required?|Yes|
|Branches|`Normalisation`|
{{< /htable >}}

## Overview

The `Process3D` node is used to process histogram data generated by the {{< node "Collect3D" >}}and expose it for visualisation and export.

## Description

Once a {{< node "Collect3D" >}} node has accumulated a histogram, the `Process3D` node can process it into something meaningful to the user, adjusting, scaling, and normalising as appropriate. It also lets axis names be assigned to the data, and converts the original histogram data into a form suitable for plotting and/or export.

Scaling, normalisation, and other operations on the data can be performed through the the node's local branch (see below).

The `Process3D` node is intended to appear at the very end of a procedure, preparing the collected data for consumption.

## Branching

The `Process*` nodes all have a branch with the "Operate" context (accessed through the hidden `Normalisation` keyword) which is always executed, and can be used to perform mathematical operations via a sequence of `Operate*` nodes.

## Options

### Source

|Keyword|Arguments|Default|Description|
|:------|:--:|:-----:|-----------|
|`SourceData`|`name`|--|{{< required-label >}} The `name` of a {{< node "Collect3D" >}} node containing the histogram data to process.|

### Labels

|Keyword|Arguments|Default|Description|
|:------|:--:|:-----:|-----------|
|`LabelValue`|`label`|`"Counts"`|Label for the value axis|
|`LabelX`|`label`|`"X"`|Label for the x axis|
|`LabelY`|`label`|`"Y"`|Label for the y axis|
|`LabelZ`|`label`|`"Z"`|Label for the z axis|

### Export

|Keyword|Arguments|Default|Description|
|:------|:--:|:-----:|-----------|
|`Export`|[`Data3DFileAndFormat`]({{< ref "data3dformat" >}})|--|File format and file name under which to save processed data.|
