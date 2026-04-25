# Codeforces Clawer — 竞技编程数据采集与可视化系统

> 从 libcurl 都装不上，到一套完整的模块化数据管道。这不仅是课设，是我写给 17 岁自己的一封情书。

## 📌 项目简介
<<<<<<< Updated upstream
本项目利用 Codeforces 官方 API，自动抓取用户比赛信息与提交记录，进行多维度统计，并生成交互式 HTML 可视化报告。支持单用户深度分析与多用户批量对比。

最初这只是程序设计课程大作业，但我没有止步于“能跑就行”。后续加入了本地缓存、赛后补题判定、双窗口难度直方图、模块化重构等进阶特性，把它从一个单文件脚本打磨成了具备工业思维的数据管道。

## 🔥 核心功能
- 读取用户 ID（交互输入 / `users.txt` 批量读入）
- 抓取比赛列表、参赛记录、排名、各题得分
- 获取全部提交记录，**自动判定赛后补题**（表格中以 √ 标记）
- 统计当前 Rating、头衔、头像、比赛次数、最高 Rating、近 180 天数据
- 用户 ID 与 Rating 数值严格按 CF 官方规则着色
- 按时间倒序展示比赛详情表格（赛事名称、赛前/赛后 Rating、排名、各题得分、补题标记）
- **多用户支持**：从 `users.txt` 批量生成个人报告与总览页 `index.html`，用户名可点击跳转
- **题目难度直方图**：统计全部历史与近 180 天 AC 题目难度分布，双柱对比展示

## 🚀 快速开始
1. 克隆本仓库
2. 用 Visual Studio 2022 打开 `OYSQ.sln`
3. 确认包含目录与库目录已正确指向 `third_party` 和 `libs`
4. 编译运行 (`Ctrl+F5`)，按提示选择模式

## 📖 开发环境
- Windows 10/11 · Visual Studio 2022 · C++17
- libcurl（已包含） · cJSON（已包含）

## 🧠 踩坑与成长（这四件事让我真正入门了工程开发）

**① 三天配通一个库：从 LNK2019 到 MinGW/MSVC 兼容转换**
完全不知道怎么配 libcurl。vcpkg 反复失败，官网预编译包是 MinGW 格式，和 MSVC 八字不合。我用 `dumpbin /exports` 导出符号表，手写 `.def` 文件，再用 `lib` 命令生成兼容的 `.lib`。当控制台吐出 `libcurl version: 8.19.0` 时，我亲手理解了什么叫“静态库与动态库的区别”。

**② 用缓存把查询加速 10 倍**
发现重复查询同一个用户时，程序又把所有比赛重新爬一遍，既慢又不尊重 API 限流。于是设计了本地缓存：首次抓取后把 JSON 存到 `cache/` 目录，下次直接读；同时用 `throttle()` 保证每两次请求间隔 ≥2 秒。`tourist` 第二次运行从 6 分钟降到了 20 秒。

**③ 与UTF-8乱码的终极对决**
这是整个项目中耗时最长、也让我对底层编码理解最深刻的一关。动态生成HTML后，浏览器里的中文全变成乱码。我试遍了几乎所有能想到的方案：在VS中用“高级保存选项”将所有源文件存为UTF-8、在HTML头部添加双重`<meta charset>`标签、手动切换浏览器编码、甚至尝试将数据与模板分离的“前后端分离”架构。问题看似顽固，但最终追溯到了根源：C++源文件在MSVC编译器下的字符集处理远比想象中复杂，硬编码的中文在不同环境间极易失真。最终的解决方案是一次务实的工程权衡——将所有对用户可见的界面文本**改用纯英文实现**。这不是妥协，而是我在理解了跨平台编码的复杂性后，为保证项目在任何环境下都100%稳定可靠而做的主动选择。这段经历让我对“字符编码”有了刻骨铭心的理解。 

**④ 模块化重构：每移一个函数编译一次**
最初所有代码挤在一个 `OYSQ.cpp` 里。想学同校学长拆成四层，结果每次一拆就报“无法解析的外部符号”。最后总结出铁律：**移动一个函数立刻编译，通过才继续；每个 `.cpp` 必须包含自己的 `.h` 和所有用到的标准库**。凌晨三点，当 `main.cpp` 里只剩 `main()` 函数时，我觉得自己像个工程师了。

## 💡 技术亮点
- **本地缓存**：二次查询耗时缩短 90%，严格遵守 API 限流
- **赛后补题智能判定**：自动区分“场内 AC”与“赛后补题”，表格中以 √ 直观展示
- **模块化架构**：网络层、解析层、统计层、渲染层四层分离，接口清晰
- **跨平台编码处理**：主动写入 UTF-8 BOM，确保中文在任意浏览器中正常显示
- **API 限流与错误处理**：`throttle()` 保证请求间隔 ≥2s；所有 cJSON 解析均有空值检查

## 📁 项目结构
```text
OYSQ/
├── main.cpp                 # 程序入口，流程调度
├── api_client.h/cpp         # 网络请求层 (libcurl封装、限流、缓存)
├── data_parser.h/cpp        # JSON解析 (cJSON封装)
├── statistics.h/cpp         # 统计计算 (近180天、难度直方图、补题判定)
├── html_render.h/cpp        # HTML报告生成 (ECharts图表、表格渲染)
├── utils.h/cpp              # 工具函数 (颜色映射、时间格式化、缓存IO)
├── data_structures.h        # 数据结构定义
├── third_party/             # 第三方库 (cJSON)
├── libs/                    # libcurl 库文件
└── users.txt                # 多用户输入样例

## 运行截图
<img width="1399" height="421" alt="image" src="https://github.com/user-attachments/assets/3c056849-d66c-468c-84df-909ce1bc398f" />

<img width="1274" height="768" alt="image" src="https://github.com/user-attachments/assets/b1ec1c41-6b9a-4f8a-8a75-ebbebd3fb876" />

<img width="1242" height="759" alt="image" src="https://github.com/user-attachments/assets/744c056a-62d7-456c-930c-d63c5e2f1ea7" />

<img width="1302" height="607" alt="image" src="https://github.com/user-attachments/assets/0b0d83de-15c3-4b23-9843-cc09e53b33a1" />

<img width="1328" height="643" alt="image" src="https://github.com/user-attachments/assets/aa74e069-33fa-4ad8-9ace-76b1b37f205f" />

<img width="1309" height="727" alt="image" src="https://github.com/user-attachments/assets/c31a0662-14a5-4215-8c98-9b6ab94eb432" />

## 未来计划
- 将数据存储迁移至 MySQL，支持持久化与复杂查询
- 引入调度工具，实现每日自动抓取与增量更新
- 尝试用简单模型预测下一场 Rating 变化
- 把数据管道搬上 Web，做成在线分析平台

## 致谢
感谢课程老师提供的选题与指导。感谢 Codeforces 开放 API。`cJSON` 来源于 DaveGamble/cJSON，`libcurl` 来自 curl 官方。开发过程中参考了社区多位前辈的开源项目与博客，在此一并致谢。

## 关于作者
我叫欧阳思琦，17 岁，就读于湘潭大学计算机相关专业。热爱用代码把数据从源头搬到屏幕上的全过程，对人工智能方向充满探索欲。如果你有数据工程、后端开发或 AI 相关的实习/研究机会，欢迎联系我聊聊。

📬 GitHub: [SiqiOuyang](https://github.com/SiqiOuyang)

---

*此项目献给那个在黑暗中硬啃配环境的自己。也献给每一个正在死磕大作业的你——你不是一个人在战斗。*
