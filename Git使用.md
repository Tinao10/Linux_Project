```bash
# 初始化本地仓库
git init

# 添加所有文件
git add .  

# 提交更改
git commit -m "初次提交"

# 添加远程仓库（使用SSH地址）
git remote add origin git@github.com:你的用户名/你的仓库.git

# 确保分支名为main（如果当前分支是master，重命名）
git branch -M main

# 推送到GitHub
git push -u origin main
```

