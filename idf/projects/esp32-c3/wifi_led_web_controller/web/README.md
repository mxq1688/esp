# ESP32-C3 LED控制器 Web界面

这个目录包含了ESP32-C3 LED控制器的独立web文件，包括HTML、CSS、JavaScript等。

## 文件结构

```
web/
├── index.html          # 主页面HTML
├── style.css           # 样式文件
├── script.js           # JavaScript功能文件
├── manifest.json       # PWA配置文件
├── sw.js              # Service Worker文件
└── README.md          # 本说明文件
```

## 优势

相比之前在C文件中嵌入HTML/CSS/JS的方式，使用独立文件有以下优势：

1. **易于维护**: 可以直接编辑HTML、CSS、JS文件，无需重新编译固件
2. **开发友好**: 可以使用现代web开发工具进行调试和开发
3. **版本控制**: 可以单独管理web界面的版本
4. **热更新**: 可以通过SPIFFS文件系统更新web界面，无需重新烧录固件
5. **代码分离**: 前端代码和后端代码完全分离，职责清晰

## 使用方法

### 1. 开发阶段

在开发web界面时，你可以：

- 直接在浏览器中打开`index.html`进行预览
- 使用VS Code等编辑器进行代码编辑
- 使用浏览器开发者工具进行调试
- 使用现代CSS和JavaScript特性

### 2. 部署到ESP32

将web文件部署到ESP32的SPIFFS文件系统：

```bash
# 使用ESP-IDF的spiffsgen工具
idf.py spiffs-image

# 或者手动上传文件到SPIFFS分区
```

### 3. 文件路径

ESP32中的文件路径映射：

- `/` → `/spiffs/index.html`
- `/style.css` → `/spiffs/style.css`
- `/script.js` → `/spiffs/script.js`
- `/manifest.json` → `/spiffs/manifest.json`
- `/sw.js` → `/spiffs/sw.js`

## 特性

### 响应式设计
- 支持PC、平板、手机等不同设备
- 触摸设备优化
- 键盘导航支持

### 现代化UI
- 毛玻璃效果（backdrop-filter）
- 流畅的动画和过渡
- 渐变背景和阴影
- 深色模式支持

### 交互功能
- 实时颜色预览
- RGB颜色滑块控制
- 亮度调节
- 预设颜色选择
- 特效切换
- 电源控制

### PWA支持
- 可安装为桌面应用
- 离线缓存
- Service Worker支持

## 开发建议

1. **CSS**: 使用现代CSS特性，如Grid、Flexbox、CSS变量等
2. **JavaScript**: 使用ES6+语法，支持模块化开发
3. **性能**: 注意动画性能，使用`transform`和`opacity`进行动画
4. **兼容性**: 考虑不同浏览器的兼容性，必要时添加polyfill
5. **测试**: 在不同设备和浏览器上测试界面效果

## 注意事项

1. 确保ESP32的SPIFFS分区足够大，能够存储所有web文件
2. 文件路径要正确，ESP32的web服务器需要正确路由到SPIFFS文件
3. 如果使用外部文件方式，需要修改`web_files.c`中的文件读取逻辑
4. 建议保留原始的嵌入方式作为备用，以防文件读取失败

## 更新日志

### v2.0
- 将web代码从C文件中提取出来
- 优化了UI设计和动画效果
- 改进了响应式设计
- 添加了触摸设备优化
- 支持深色模式
- 添加了PWA功能

### v1.0
- 基础LED控制功能
- 简单的web界面
- 基本的颜色和亮度控制
