import os

# --- Налаштування ---

# 1. Папка, з якої потрібно збирати код
search_directory = './'

# 2. Назва результуючого файлу
output_filename = 'combined_lab_code.txt'

# 3. Розширення файлів, які потрібно включити
include_extensions = ('.c', '.cpp', '.h', '.hpp')

# --- Логіка скрипта ---

print(f"Starting code bundle process...")
print(f"Searching in: {os.path.abspath(search_directory)}")
print(f"Output file: {output_filename}\n")

file_count = 0

try:
    # Відкриваємо результуючий файл для запису
    with open(output_filename, 'w', encoding='utf-8') as outfile:
        
        # Рекурсивно обходимо директорію 'Core'
        for root, dirs, files in os.walk(search_directory):
            for file in files:
                # Перевіряємо, чи має файл потрібне розширення
                if file.endswith(include_extensions):
                    file_count += 1
                    filepath = os.path.join(root, file)
                    normalized_path = filepath.replace('\\', '/')
                    
                    print(f"  -> Adding: {normalized_path}")
                    
                    # 1. Записуємо заголовок з назвою файлу
                    outfile.write(f"// === {normalized_path} ===\n\n")
                    
                    # 2. Намагаємося прочитати вміст файлу
                    file_content = None
                    try:
                        # Спочатку пробуємо UTF-8
                        with open(filepath, 'r', encoding='utf-8') as infile:
                            file_content = infile.read()
                    except UnicodeDecodeError:
                        # Якщо не вийшло, пробуємо latin-1 (часто для Windows)
                        try:
                            with open(filepath, 'r', encoding='latin-1') as infile:
                                file_content = infile.read()
                            print(f"    [Info] Read {normalized_path} as latin-1.")
                        except Exception as e:
                            print(f"    [WARNING] Could not read file {filepath}: {e}")
                            outfile.write(f"// [Error reading file: {e}]\n")
                    except Exception as e:
                        print(f"    [WARNING] Could not read file {filepath}: {e}")
                        outfile.write(f"// [Error reading file: {e}]\n")

                    # 3. Якщо вміст успішно прочитано, записуємо його
                    if file_content is not None:
                        outfile.write(file_content)
                    
                    # 4. Додаємо роздільник для читабельності
                    outfile.write(f"\n\n// {'='*76} //\n\n")

    print(f"\nSuccess! Combined {file_count} files into '{output_filename}'.")

except FileNotFoundError:
    print(f"\n[ERROR] Directory not found: '{search_directory}'")
    print("Please make sure you run this script from your project's root directory")
except Exception as e:
    print(f"\n[ERROR] An unexpected error occurred: {e}")