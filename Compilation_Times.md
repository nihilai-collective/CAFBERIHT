# Compilation Times (Intel i9 14900kf)

| Compiler | Compilation Time | Hardware | Base Count | Notes |
|----------|------------------|----------|------------|-------|
| **MSVC 19.44.35222** | **1.30** | Windows, x64 | 100 | Full template instantiation + codegen |
| **MSVC 19.44.35222** | **3.61s** | Windows, x64 | 1000 | Full template instantiation + codegen |
| **MSVC 19.44.35222** | **4m38s** | Windows, x64 | 10000 | Full template instantiation + codegen |
| **Clang 19.1.1** | **1.12s** | WSL/Linux, x64 | 100 | Full template instantiation + codegen |
| **Clang 19.1.1** | **3.26s** | WSL/Linux, x64 | 1000 | Full template instantiation + codegen |
| **Clang 19.1.1** | **3m37s** | WSL/Linux, x64 | 10000 | Full template instantiation + codegen |
| **GCC 14.2.0** | **0.96s** | WSL/Linux, x64 | 100 | Full template instantiation + codegen |
| **GCC 14.2.0** | **4.90s** | WSL/Linux, x64 | 1000 | Full template instantiation + codegen |
| **GCC 14.2.0** | **54m38s** | WSL/Linux, x64 | 10000 | Full template instantiation + codegen |