import matplotlib.pyplot as plt

# Data for the number of threads and speedup for 3 algorithms
threads = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14]

# Speedup values for each algorithm
malloc_speedup = [1.0000, 1.1132, 1.3186, 1.4446, 1.3653, 1.4878, 1.0127, 1.1150, 1.3119, 1.2430, 1.3765, 1.2218, 1.3580, 1.5099]
hoard_speedup = [1.0000, 0.2516, 0.3629, 0.4250, 1.0171, 1.1889, 0.8714, 1.3817, 1.0711, 1.1423, 1.1953, 1.8487, 2.2431, 2.8917]
my_mem_alloc_speedup = [1.0000, 5.1100, 11.3300, 17.2500, 20.3600, 22.6100, 22.6800, 25.8700, 26.2100, 26.4700, 27.7300, 28.5400, 29.8400, 30.3000]

# Create the plot
plt.figure(figsize=(10, 6))

# Plot the speedup values for each algorithm
plt.plot(threads, malloc_speedup, marker='o', linestyle='-', color='#1f77b4', label='Malloc', markersize=8, linewidth=2)
plt.plot(threads, hoard_speedup, marker='s', linestyle='--', color='#ff7f0e', label='Hoard', markersize=8, linewidth=2)
plt.plot(threads, my_mem_alloc_speedup, marker='^', linestyle='-.', color='#2ca02c', label='My_mem_alloc', markersize=8, linewidth=2)

# Adding labels and title with larger font
plt.xlabel('Number of Threads', fontsize=12, fontweight='bold')
plt.ylabel('Speedup', fontsize=12, fontweight='bold')
plt.title('Comparison of Memory Allocators for Scalability', fontsize=14, fontweight='bold')

# Set x-axis to show all values with larger font
plt.xticks(threads, fontsize=10)
plt.yticks(fontsize=10)

# Add a light grid for better visualization
plt.grid(True, which='both', linestyle='--', linewidth=0.5, alpha=0.7)

# Add a legend with a larger font size
plt.legend(fontsize=10)

# Tight layout for better spacing
plt.tight_layout()

# Save the plot as a PNG image
plt.savefig('scalability_comparison.png', dpi=300, bbox_inches='tight')

# Show the plot
# plt.show()
