// Copyright (C) 2026 Intel Corporation
// SPDX-License-Identifier: MIT
import { type Widget, WidgetType, generateKey } from './widget'
import { makeDefaultWidgetMetric } from './widget-metric';
import { type QualifiedMetric } from './qualified-metric';
import { type RgbaColor } from './color';

export interface Qos extends Widget {
    showLabel: boolean,
    fontSize: number,
    fontColor: RgbaColor,
    backgroundColor: RgbaColor,
}

export function makeDefaultQos(metric: QualifiedMetric): Qos {
    return {
        key: generateKey(),
        metrics: [makeDefaultWidgetMetric(metric)],
        widgetType: WidgetType.Qos,
        showLabel: true,
        fontSize: 22,
        fontColor: {
            r: 205,
            g: 211,
            b: 233,
            a: 1
        },
        backgroundColor: {
            r: 45,
            g: 50,
            b: 96,
            a: 0.4
        },
    };
}
