#!/usr/bin/env python3
"""Plot performance metrics (averages and stddevs) captured from the program.

Creates two plots:
1) Average processing times per repetition for capture, detection, feedback, and total.
2) Stddev of processing times per repetition for the same modules.

Saves plots to 'performance_times.png' and 'performance_stddevs.png' in the current
working directory and also displays them.

This script is self-contained and uses the values supplied in the attachment.
It masks extremely large outliers (e.g. integer overflow) so the plots remain readable.
"""
import math
import numpy as np
import matplotlib.pyplot as plt
from typing import List, Optional


def mask_outliers(values: List[float], threshold: float = 1e9) -> np.ndarray:
    """Convert list to numpy array and replace values with NaN when abs(value) > threshold.

    This protects plots from extreme outliers like integer overflow values.
    """
    arr = np.array(values, dtype=float)
    mask = np.abs(arr) > threshold
    if mask.any():
        print(f"Masking {mask.sum()} outlier(s) greater than {threshold}")
        arr[mask] = np.nan
    return arr


def main() -> None:
    # Repetition index (1..10)
    reps = np.arange(1, 11)

    # Averages (ms) extracted from the attachment
    total_avg = [44.54, 44.19, 44.96, 44.50, 44.09, 43.13, 45.09, 46.02, 44.20, 44.16]
    capture_avg = [27.81, 27.89, 27.86, 27.87, 27.90, 27.89, 27.87, 27.89, 27.91, 27.91]
    detection_avg = [5.63, 5.40, 5.73, 5.51, 5.38, 5.50, 5.74, 5.61, 5.37, 5.36]
    non_verbal_feedback_avg = [8.19, 7.99, 8.38, 8.18, 7.98, 8.06, 8.48, 8.20, 7.99, 7.95]


    total_std = [2.80, 2.50, 2.54, 2.45, 2.40, 3.24, 2.53, 2.52, 2.35, 2.36]
    capture_std = [0.82, 0.70, 0.89, 0.71, 0.77, 0.82, 0.78, 0.86, 0.73, 0.69]
    detection_std = [0.73, 0.71, 0.77, 0.69, 0.64, 0.72, 0.74, 0.79, 0.62, 0.64]
    non_verbal_feedback_std = [1.24, 1.27, 1.27, 1.28, 1.22, 1.24, 1.32, 1.25, 1.24, 1.22]

    # Convert other lists to numpy arrays for consistency
    capture_avg = np.array(capture_avg)
    detection_avg = np.array(detection_avg)
    non_verbal_feedback_avg = np.array(non_verbal_feedback_avg)

    capture_std = np.array(capture_std)
    detection_std = np.array(detection_std)
    non_verbal_feedback_std = np.array(non_verbal_feedback_std)

    # Sanitize arrays to avoid plotting extreme outliers
    total_avg = mask_outliers(total_avg)
    capture_avg = mask_outliers(capture_avg.tolist())
    detection_avg = mask_outliers(detection_avg.tolist())
    non_verbal_feedback_avg = mask_outliers(non_verbal_feedback_avg.tolist())

    total_std = mask_outliers(total_std)
    capture_std = mask_outliers(capture_std.tolist())
    detection_std = mask_outliers(detection_std.tolist())
    non_verbal_feedback_std = mask_outliers(non_verbal_feedback_std.tolist())

    # Helper to create a 2x2 subplot figure for a set of series
    def plot_2x2(values_list, titles, y_label, out_base_name, general_title: Optional[str] = None):
        fig, axs = plt.subplots(2, 2, figsize=(12, 8), sharex=True)
        axs = axs.flatten()
        for ax, vals, title in zip(axs, values_list, titles):
            ax.plot(reps, vals, marker='o')
            ax.set_title(title)
            ax.set_xlabel('Iteración')
            ax.set_ylabel(y_label)
            ax.set_xticks(reps)
            ax.grid(True, linestyle=':', alpha=0.6)

        # Add a general title if provided and make room for it
        if general_title:
            fig.suptitle(general_title, fontsize=16)
            plt.tight_layout(rect=(0, 0, 1, 0.95))
        else:
            plt.tight_layout()
        png_name = f"{out_base_name}.png"
        svg_name = f"{out_base_name}.svg"
        fig.savefig(png_name, dpi=150)
        fig.savefig(svg_name, dpi=150, format='svg')
        print(f"Saved {out_base_name} plots to {png_name} and {svg_name}")

    # Plot 1: averages -> one module per subplot
    avg_values = [capture_avg, detection_avg, non_verbal_feedback_avg, total_avg]
    avg_titles = ['Módulo de captura', 'Módulo de detección', 'Módulo de retroalimentación', 'Total']
    plot_2x2(avg_values, avg_titles, 'Tiempo de procesamiento promedio (ms)', 'tiempo_promedio_no_verbal')

    # Plot 2: stddevs -> one module per subplot
    std_values = [capture_std, detection_std, non_verbal_feedback_std, total_std]
    std_titles = ['Módulo de captura', 'Módulo de detección', 'Módulo de retroalimentación', 'Total']
    plot_2x2(std_values, std_titles, 'Desviación estándar (ms)', 'desviacion_estandar_no_verbal')

    # Show plots interactively (may fail in headless environments)
    try:
        plt.show()
    except Exception:
        print("Unable to show plots interactively (likely headless). Images were saved to disk.")


if __name__ == '__main__':
    main()
