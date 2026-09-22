import pandas as pd
import numpy as np
import random
import string
import time
from google.colab import files

# ==============================================================================
# 1. USER INPUT & CONFIGURATION
# ==============================================================================
print("=== Google Colab Dataset Generator ===")
num_entries_input = input("Enter the total number of entries required (e.g., 1000, 100000, 1000000): ")
num_entries = int(num_entries_input.replace(',', '').strip())

print(f"\n[+] Generating dataset with {num_entries:,} randomized records...")
start_time = time.time()

# ==============================================================================
# 2. RANDOMIZED DATA GENERATION
# ==============================================================================

# Generates fast alphanumeric random strings (e.g., 'aX9k2L1P')
def generate_random_strings(count, length=8):
    chars = string.ascii_letters + string.digits
    return [''.join(random.choices(chars, k=length)) for _ in range(count)]

# Primary Keys (Sequential Key Identifiers: Key_1, Key_2, ...)
keys = [f"Key_{i}" for i in range(1, num_entries + 1)]

# Randomized String Data (e.g., Names/Text hashes)
random_strings = generate_random_strings(num_entries, length=8)

# Randomized Decimal Float Data (e.g., 10.00 to 9999.99)
random_floats = np.round(np.random.uniform(10.00, 9999.99, size=num_entries), 2)

# Randomized Integer Data (e.g., 1000 to 999999)
random_ints = np.random.randint(1000, 999999, size=num_entries)

# ==============================================================================
# 3. DATAFRAME CREATION & EXPORT
# ==============================================================================
# Construct DataFrame
df = pd.DataFrame({
    'Key': keys,
    'String_Value': random_strings,
    'Float_Value': random_floats,
    'Int_Value': random_ints
})

# Export DataFrame to CSV
filename = f"dataset_{num_entries}_entries.csv"
df.to_csv(filename, index=False)

end_time = time.time()
print(f"[✓] Successfully generated and exported DataFrame in {end_time - start_time:.2f} seconds.")

# ==============================================================================
# 4. PREVIEW & AUTOMATIC DOWNLOAD
# ==============================================================================
print("\n--- DataFrame Preview (First 5 Rows) ---")
print(df.head())

print("\n--- DataFrame Info ---")
print(f"Total Rows: {len(df):,}")
print(f"Total Columns: {list(df.columns)}")

# Trigger browser download of the CSV file from Google Colab
print(f"\n[+] Triggering download for '{filename}'...")
files.download(filename)