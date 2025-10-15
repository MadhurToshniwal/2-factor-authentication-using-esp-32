# Contributing to 2FA Authentication using ESP32

Thank you for your interest in contributing to this project! This document provides guidelines and information for contributing.

## 🤝 How to Contribute

### Reporting Issues

1. **Check existing issues** first to avoid duplicates
2. **Use the issue template** when creating new issues
3. **Provide detailed information**:
   - ESP32 board type and version
   - Arduino IDE version
   - Operating system
   - Steps to reproduce
   - Expected vs actual behavior
   - Screenshots/serial output if applicable

### Suggesting Enhancements

1. **Open an issue** with the enhancement label
2. **Describe the enhancement** in detail
3. **Explain the use case** and benefits
4. **Provide mockups** or examples if applicable

### Pull Requests

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Make your changes** following the coding standards
4. **Test thoroughly** on actual hardware
5. **Update documentation** if needed
6. **Commit with clear messages**: `git commit -m "Add amazing feature"`
7. **Push to your fork**: `git push origin feature/amazing-feature`
8. **Create a Pull Request**

## 🛠️ Development Setup

### Prerequisites
- Node.js (v16 or higher)
- Arduino IDE with ESP32 support
- ESP32 development board
- Git

### Local Development
```bash
# Clone your fork
git clone https://github.com/yourusername/2-factor-authentication-using-esp-32.git
cd 2-factor-authentication-using-esp-32

# Backend setup
cd backend
npm install
npm start

# Frontend setup (new terminal)
cd frontend
npm install
npm start

# Upload firmware to ESP32 using Arduino IDE
```

## 📝 Coding Standards

### JavaScript/Node.js
- Use ES6+ features
- Follow standard JavaScript style guide
- Add JSDoc comments for functions
- Use meaningful variable names
- Handle errors properly

### React
- Use functional components with hooks
- Follow React best practices
- Add PropTypes for component props (when applicable)
- Keep components small and focused

### Arduino C++
- Follow Arduino coding style
- Use meaningful function and variable names
- Add comments for complex logic
- Include error handling
- Use `Serial.println()` for debugging output

### Git Commit Messages
- Use present tense ("Add feature" not "Added feature")
- Keep first line under 50 characters
- Reference issues and pull requests when applicable
- Use conventional commit format when possible:
  ```
  feat: add new MQTT authentication
  fix: resolve WiFi connection timeout
  docs: update README with new setup steps
  ```

## 🧪 Testing

### Hardware Testing
- Test on actual ESP32 hardware
- Verify WiFi connectivity with different networks
- Test MQTT communication
- Validate LED and button functionality
- Check serial output for errors

### Software Testing
- Test all API endpoints
- Verify frontend functionality
- Check WebSocket connections
- Test device registration flow
- Validate 2FA confirmation process

### Before Submitting PR
- [ ] Code compiles without warnings
- [ ] Hardware functionality tested
- [ ] Documentation updated
- [ ] No sensitive information in commits
- [ ] Git history is clean

## 📚 Documentation

### Code Documentation
- Add comments for complex logic
- Document API endpoints
- Include usage examples
- Update README when adding features

### Hardware Documentation
- Document pin connections
- Include wiring diagrams for new features
- Update component requirements
- Add troubleshooting steps

## 🏷️ Issue Labels

- `bug`: Something isn't working
- `enhancement`: New feature or request  
- `documentation`: Improvements or additions to docs
- `good first issue`: Good for newcomers
- `help wanted`: Extra attention is needed
- `hardware`: Hardware-related issues
- `firmware`: ESP32 firmware issues
- `frontend`: React frontend issues
- `backend`: Node.js backend issues

## 🚫 What Not to Include

- Sensitive information (API keys, passwords)
- Large binary files
- Temporary or build files
- IDE-specific files
- Personal configuration files

## 📄 License

By contributing, you agree that your contributions will be licensed under the MIT License.

## ❓ Questions?

Feel free to:
- Open an issue for questions
- Email: madhurtoshniwal03@gmail.com
- Start a discussion in GitHub Discussions

## 🙏 Recognition

Contributors will be recognized in:
- README.md acknowledgments
- Release notes
- Project documentation

Thank you for contributing! 🎉