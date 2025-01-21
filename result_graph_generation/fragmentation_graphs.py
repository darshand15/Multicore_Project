import matplotlib.pyplot as plt
import numpy as np

# Data for Fragmentation ratio
categories = ['Malloc', 'Hoard', 'My_Mem_Alloc']
fragmentation_ratio = [0.06243318907816552, 0.08929032051819973, 4.219763357588981e-06]

# Create the figure and axis
plt.figure(figsize=(10, 6))

# Create the bar graph with thinner bars
bars = plt.bar(categories, fragmentation_ratio, color=['#1f77b4', '#ff7f0e', '#2ca02c'], width=0.5, edgecolor='black')

# Adding title and labels with better font weight
plt.xlabel('Memory Allocator', fontsize=12, fontweight='bold')
plt.ylabel('Fragmentation Ratio', fontsize=12, fontweight='bold')
plt.title('Comparison of Memory Allocators for Fragmentation', fontsize=14, fontweight='bold')

# Set logarithmic scale for y-axis
plt.yscale('log')

# Add values on top of the bars with better placement
for bar in bars:
    yval = bar.get_height()
    plt.text(bar.get_x() + bar.get_width() / 2, yval, f'{yval:.6e}', ha='center', va='bottom', fontsize=10, fontweight='bold')

# Adding grid for better visualization
plt.grid(True, axis='y', linestyle='--', linewidth=0.5, alpha=0.7)

# Adjust layout for better spacing and readability
plt.tight_layout()

# Save the plot as a PNG image
plt.savefig('fragmentation_comparison.png', dpi=300, bbox_inches='tight')

# Show the plot
# plt.show()
