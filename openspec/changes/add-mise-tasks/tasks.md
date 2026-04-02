## 1. Add `.mise.toml`

- [x] 1.1 Create `.mise.toml` at the repository root with a `[env]` section that sets `_.path = ["{{config_root}}/build"]`
- [x] 1.2 Add a `[tasks.setup]` entry with `description = "Configure the build directory"` and `run = "meson setup build"`
- [x] 1.3 Add a `[tasks.build]` entry with `description = "Build the project"` and `run = "ninja -C build"`
- [x] 1.4 Add a `[tasks.test]` entry with `description = "Run tests"` and `run = "meson test -C build"`
- [x] 1.5 Verify the file is valid TOML (no syntax errors)
