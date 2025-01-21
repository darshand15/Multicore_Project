import matplotlib.pyplot as plt

# Data: number of threads and corresponding execution times
threads = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14]
execution_times = [0.00141717, 0.00407351, 0.0058006, 0.00754753, 0.00899314,
                   0.0112511, 0.0112834, 0.0133583, 0.0148479, 0.0191609, 
                   0.0216021, 0.0266111, 0.0277014, 0.024346]

# Create the plot
plt.figure(figsize=(10, 6))  # Increase figure size for better readability

# Plot the line with improved style
plt.plot(threads, execution_times, marker='o', linestyle='-', color='#1f77b4', label='Execution Time', markersize=8, linewidth=2)

# Adding labels and title with larger font
plt.xlabel('Number of Threads', fontsize=12, fontweight='bold')
plt.ylabel('Execution Time (in seconds)', fontsize=12, fontweight='bold')
plt.title('Execution Time vs Number of Threads', fontsize=14, fontweight='bold')

# Set x-axis to show all values with larger font
plt.xticks(threads, fontsize=10)
plt.yticks(fontsize=10)

# Add a light grid for better visualization
plt.grid(True, which='both', linestyle='--', linewidth=0.5, alpha=0.7)

# Add labels to highlight the values of the points, with better spacing
for i, (x, y) in enumerate(zip(threads, execution_times)):
    # Place the label slightly above each point
    plt.text(x, y + 0.0005, f'{y:.6f}', ha='center', va='bottom', fontsize=7, color='black', fontweight='bold')

# Add a legend with a larger font size
plt.legend(fontsize=10)

# Tight layout for better spacing
plt.tight_layout()

# Save the plot as a PNG image
plt.savefig('execution_time_vs_threads_improved.png', dpi=300, bbox_inches='tight')

# Show the plot
# plt.show()
