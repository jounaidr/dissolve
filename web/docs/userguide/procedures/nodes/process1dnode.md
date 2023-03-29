---
title: Process1D (Node)
linkTitle: Process1D
description: Process a 1D histogram
---

{{< htable >}}
| | |
|-|-|
|Context|Analysis|
|Name Required?|Yes|
|Branches|`Normalisation`|
{{< /htable >}}

## Overview

The `Process1D` node is used to process histogram data generated by the {{< node "Collect1D" >}}and expose it for visualisation and export.

## Description

Once a {{< node "Collect1D" >}} node has accumulated a histogram, the `Process1D` node can process it into something meaningful to the user, adjusting, scaling, and normalising as appropriate. It also lets axis names be assigned to the data, and converts the original histogram data into a form suitable for plotting and/or export.

Scaling, normalisation, and other operations on the data can be performed through the the node's local branch (see below).

The `Process1D` node is intended to appear at the very end of a procedure, preparing the collected data for consumption.

## Branching

The `Process*` nodes all have a branch with the "Operate" context (accessed through the hidden `Normalisation` keyword) which is always executed, and can be used to perform mathematical operations via a sequence of `Operate*` nodes.

## Options

### Source

|Keyword|Arguments|Default|Description|
|:------|:--:|:-----:|-----------|
|`Instantaneous`|`bool`|`false`|Whether the processed data should reflect the accumulated histogram data (`false`) or the "instantaneous" data from the last iteration only (`true`).|
|`SourceData`|`name`|--|{{< required-label >}} The `name` of a {{< node "Collect1D" >}} node containing the histogram data to process.|

### Labels

|Keyword|Arguments|Default|Description|
|:------|:--:|:-----:|-----------|
|`LabelValue`|`label`|`"Y"`|Label for the value axis|
|`LabelX`|`label`|`"X"`|Label for the x axis|

### Export

|Keyword|Arguments|Default|Description|
|:------|:--:|:-----:|-----------|
|`Export`|[`Data1DFileAndFormat`]({{< ref "data1dformat" >}})|--|File format and file name under which to save processed data.|