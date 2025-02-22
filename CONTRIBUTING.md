# Contributing to MemoryList

Thank you for considering contributing to MemoryList! This document provides guidelines for contributing.

## How to Contribute

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Development Guidelines

### Code Style
- Use consistent indentation (4 spaces)
- Follow existing naming conventions
- Add comments for complex logic
- Include documentation for public methods

### Testing
- Add tests for new features
- Ensure all tests pass before submitting PR
- Follow existing test patterns in `test_memory_list.cpp`
- Test edge cases and buffer boundaries

### Documentation
- Update README.md if adding new features
- Document public API changes
- Include example usage when appropriate

## Performance Considerations
- Keep buffer sizes ESP32-friendly (512 bytes default)
- Minimize memory allocations
- Consider SD card write cycles
- Test with large datasets

## Bug Reports
Include:
- ESP32 board type
- Arduino/ESP-IDF version
- SD card details
- Minimal reproduction code
- Expected vs actual behavior