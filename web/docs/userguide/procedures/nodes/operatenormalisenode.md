---
title: OperateNormalise (Node)
linkTitle: OperateNormalise
description: Perform normalisation of numerical data
---

{{< htable >}}
| | |
|-|-|
|Context|Operate|
|Name Required?|No|
|Branches|--|
{{< /htable >}}

## Overview

The `OperateNormalise` node normalises a data-space sum to a specific value. 

## Description

In the "Operate" context all `Operate*` nodes are passed data of some dimensionality (e.g. from one of the `Process*` nodes) which is then subjected to some kind of mathematical operation.

The `OperateNormalise` node calculates the sum (or absolute sum) of the supplied data, and divides all values by the sum in order to give the normalised data whose sum equals 1.0. The data is then multiplied by the supplied value (which also defaults to 1.0) to give a sum other than 1.0. The target data may be of any dimensionality.

## Options

### Inputs

|Keyword|Arguments|Default|Description|
|:------|:--:|:-----:|-----------|
|`Value`|`double`|`1.0`|Constant value to normalise to.|
|`Absolute`|`bool`|`false`|Whether to use the absolute sum of values rather than the direct sum.|
