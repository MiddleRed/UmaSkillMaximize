# UmaSkillMaximize
最大化技能评价点计算器
## 如何使用
*请务必逐字阅读完整*
1. 下载页面右边 `release` （发布）中的zip，并将压缩包里所有文件解压到同一文件夹（请确保文件夹目录没有中文字符）  
2. 在Bwiki的[评分计算器](https://wiki.biligame.com/umamusume/%E8%AF%84%E5%88%86%E8%AE%A1%E7%AE%97%E5%99%A8)中选择你已经得到的技能，并输入马的适性（必须输入，技能评分与适性有关）以及继承技数量，其他属性可以不输入  
在选择技能时你应该遵守以下规则：  
如果是有单双圈之分的技能（例：`右回り○`），只需要选择**单圈技能**，不用选择双圈。若你得到了这种技能的金技能（例：`良バ場の鬼`），则只需要选择**金技能**，不用选择其下位技能。  
如果没有单双圈之分的技能，正常选择即可。若你得到了这种技能的金技能，只需要选择**金技能**，不用选择其下位技能。  
**计算器默认你选择的技能全都没有点过，如果是已经点过的技能请不要选择，包括其对应的上位技能**  
选择技能时，不需要输入紫色技能
3. 选择完毕后，在评分计算器下面点击“生成分享码”，并复制其生成的内容  
4. 打开下载的计算器(Calc.exe)，输入刚刚复制的分享码，并之后按照提示输入对应的内容  
**注意：在输入技能Hint时，请注意金技能的Hint和其下位技能的Hint没有关系，请分别输入，不要混淆。** 输入有单双圈之分的技能时，只需要输入一次Hint，计算器会自动将此Hint应用到单圈技能上。如果技能没有Hint值，请输入`0`。如果显示技能Hint达到了最大值，请输入`5`  
5. 得到结果  

因为Bwiki选择显示全部技能时，技能的排序方法是对着游戏来的，前后顺序没有倒，所以选起来会快很多。输入Hint的时候技能排序也是对着游戏来的，省去了翻来翻去的麻烦
## 报错自查  
· 无效的分享码：请检查分享码是否有漏复制或缺损的情况，然后重新输入  
· 无法打开 `master.mdb` 文件：此计算器仅适用于Windows平台DMM客户端的赛马娘，需要读取游戏目录下的数据库文件。请手动打开游戏资源文件夹 `C:\Users\[用户名]\AppData\LocalLow\Cygames\umamusume` ，检查是否存在 `master.mdb` 文件。如果确实存在但是计算器无法读取，你可以尝试手动将文件路径复制到计算器里。或者，请尝试使用第三方 `master.mdb` 文件  
· 错误：预想的查询结果只有一条，而... ：数据库内容有错误，有很大可能是数据库自身问题，请尝试使用第三方 `master.mdb` 文件  
· `SQL error` ：数据库错误，有很大可能是数据库不完整或者输入内容有问题，请尝试在游戏内检查资源完整性，或者使用第三方 `master.mdb` 文件  
· 其他的一些稀奇古改的错误（无尽刷屏等）：有可能是你的系统用户名或文件路径存在中文导致内部错误，请尝试把系统用户名改成英文，或者手动将 `master.mdb` 和计算器单独复制到没有中文的文件目录下，然后再打开
## 使用第三方数据库
因为各种原因，游戏自带的数据库 `master.mdb` 可能会出现问题。计算器提供了两种使用第三方数据库的方式：  
· 第一种：计算器内提示输入第三方数据库文件地址，复制数据库文件地址到计算器内即可  
· 第二种：使用命令行，`cd` 到解压计算器的文件目录下，并在命令行中输入 `Calc.exe [第三方数据库文件地址]` 。或者，可以直接输入 `[Calc.exe文件地址] [第三方数据库文件地址]`，计算器将会启动并自动使用第三方的数据库文件  
在下载的zip中会自带截止到发行时最新的原版 `master.mdb` 数据库。如果zip内的数据库过时了，可以考虑使用[汉化的数据库](https://ngabbs.com/read.php?tid=30170552)，效果是一样的，目前是实时更新
## Bug反馈  
如果遇到Bug，请提交issue，或者在nga帖子中回复。如果是遇到程序本体的问题，请尝试先按照以上处理错误的方法自行处理，如果无法处理请仔细说明报错的问题；如果是遇到程序计算的问题，请一并附上分享码、计算器内所有的内容，以及存在的问题，如有必要，也请截屏附上游戏内的选择技能界面的所有技能   
## 其他
可能日后会做出web端以及支持Msgpack输入，方便技能选择。
特别感谢[@Ealvn](https://space.bilibili.com/5418144/)提供评分计算器分享码算法
