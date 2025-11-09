#!/usr/bin/env python3
"""Graficar métricas de rendimiento (promedios y desviaciones estándar) para el modo
de retroalimentación VERBAL.

Genera dos figuras (cada una con una cuadrícula de 2x2 subplots):
1) Tiempos de procesamiento promedio por repetición para captura, detección,
   retroalimentación y total.
2) Desviación estándar de los tiempos por repetición para los mismos módulos.

Guarda las figuras en archivos con base ``performance_times_verbal`` y
``performance_stddevs_verbal`` en el directorio de trabajo actual y también intenta
mostrarlas en pantalla.

Este script está basado en el script de modo no verbal pero usa las estadísticas
correspondientes al modo verbal.
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

    # VERBAL mode averages (ms) -- derived from measured output
    total_avg = [37.90, 37.56, 38.58, 38.29, 38.85, 38.64, 38.12, 38.32, 38.70, 38.92]
    capture_avg = [27.88, 27.89, 27.86, 27.87, 27.90, 27.89, 27.89, 27.88, 27.91, 27.89]
    detection_avg = [5.22, 5.24, 5.23, 5.21, 5.20, 5.25, 5.25, 5.24, 5.26, 5.25]
    feedback_avg = [1.04, 1.07, 1.07, 1.07, 1.11, 1.07, 1.08, 1.06, 1.09, 1.10]

    total_std = [2.55, 2.83, 1.92, 2.06, 1.87, 1.95, 1.23, 2.04, 2.15, 2.02]
    capture_std = [0.88, 0.87, 0.86, 0.85, 0.89, 0.86, 0.84, 0.87, 0.93, 0.84]
    detection_std = [0.57, 0.58, 0.56, 0.55, 0.59, 0.60, 0.65, 0.61, 0.64, 0.64]
    feedback_std = [0.20, 0.30, 0.26, 0.26, 0.31, 0.26, 0.27, 0.24, 0.31, 0.29]

    # Convert other lists to numpy arrays for consistency
    capture_avg = np.array(capture_avg)
    detection_avg = np.array(detection_avg)
    feedback_avg = np.array(feedback_avg)

    capture_std = np.array(capture_std)
    detection_std = np.array(detection_std)
    feedback_std = np.array(feedback_std)

    # Sanitize arrays to avoid plotting extreme outliers
    total_avg = mask_outliers(total_avg)
    capture_avg = mask_outliers(capture_avg.tolist())
    detection_avg = mask_outliers(detection_avg.tolist())
    feedback_avg = mask_outliers(feedback_avg.tolist())

    total_std = mask_outliers(total_std)
    capture_std = mask_outliers(capture_std.tolist())
    detection_std = mask_outliers(detection_std.tolist())
    feedback_std = mask_outliers(feedback_std.tolist())

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
    avg_values = [capture_avg, detection_avg, feedback_avg, total_avg]
    avg_titles = ['Módulo de captura', 'Módulo de detección', 'Módulo de retroalimentación', 'Total']
    plot_2x2(
        avg_values,
        avg_titles,
        'Tiempo de procesamiento promedio (ms)',
        'tiempo_promedio_verbal'
    )

    # Plot 2: stddevs -> one module per subplot
    std_values = [capture_std, detection_std, feedback_std, total_std]
    std_titles = ['Módulo de captura', 'Módulo de detección', 'Módulo de retroalimentación', 'Total']
    plot_2x2(std_values, std_titles, 'Desviación estándar (ms)', 'desviacion_estandar_verbal')

    # Show plots interactively (may fail in headless environments)
    try:
        plt.show()
    except Exception:
        print("Unable to show plots interactively (likely headless). Images were saved to disk.")


if __name__ == '__main__':
    main()
