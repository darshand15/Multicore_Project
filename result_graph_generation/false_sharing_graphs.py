import matplotlib.pyplot as plt
import numpy as np

# Data for the execution times
algorithms = ['Malloc', 'Hoard', 'My_Mem_Alloc']
active_false_sharing = [0.00229852, 0.00203975, 0.00239038]
passive_false_sharing = [0.00494011, 0.00644643, 0.0021921]

# X-axis position for each algorithm
x = np.arange(len(algorithms))

# Create the plot
plt.figure(figsize=(10, 6))

# Plot the lines for Active_False_Sharing and Passive_False_Sharing
plt.plot(x, active_false_sharing, marker='o', linestyle='-', color='#1f77b4', label='Active_False_Sharing', markersize=8, linewidth=2)
plt.plot(x, passive_false_sharing, marker='s', linestyle='--', color='#ff7f0e', label='Passive_False_Sharing', markersize=8, linewidth=2)

# Adding labels and title with larger font
plt.xlabel('Memory Allocator', fontsize=12, fontweight='bold')
plt.ylabel('Execution Time (in seconds)', fontsize=12, fontweight='bold')
plt.title('Comparison of Memory Allocators for False Sharing', fontsize=14, fontweight='bold')

# Set x-ticks to be the algorithm names
plt.xticks(x, algorithms, fontsize=10)

# Set y-ticks with appropriate range for execution times
plt.yticks(fontsize=10)

# Add a legend
plt.legend(fontsize=10)

# Highlighting the values on the graph with better placement
for i, value in enumerate(active_false_sharing):
    # Move label a bit higher based on the value to avoid overlap
    plt.text(x[i], value + 0.0001, f'{value:.6f}', ha='center', va='bottom', fontsize=10, color='#1f77b4', fontweight='bold')

for i, value in enumerate(passive_false_sharing):
    # Move label a bit lower based on the value to avoid overlap
    plt.text(x[i], value - 0.0001, f'{value:.6f}', ha='center', va='top', fontsize=10, color='#ff7f0e', fontweight='bold')

# Adding a light grid for better visualization
plt.grid(True, which='both', linestyle='--', linewidth=0.5, alpha=0.7)

# Tight layout for better spacing
plt.tight_layout()

# Save the plot as a PNG image
plt.savefig('false_sharing_comparison.png', dpi=300, bbox_inches='tight')

# Show the plot
# plt.show()
