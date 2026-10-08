# -*- coding: utf-8 -*-
"""V6 系统架构图 + AI 数据流图 (PNG)，matplotlib 绘制"""
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch, Rectangle
from matplotlib.lines import Line2D

plt.rcParams['font.family'] = 'Microsoft YaHei'
plt.rcParams['axes.unicode_minus'] = False

OUT = r"C:\天津华铁科为车载式铁路周边环境巡检\images"

C_ROOF='#dbeafe'; C_CAB='#f0fdf4'; C_DAMP='#fef3c7'; C_SW='#f3e8ff'
C_POWER='#fee2e2'; C_TEXT='#0f172a'; C_AI='#a855f7'; C_PRIMARY='#0ea5e9'

def box(ax,x,y,w,h,text,fill,edge,fs=10,wt='normal',tc=None):
    p=FancyBboxPatch((x,y),w,h,boxstyle="round,pad=0.02,rounding_size=0.08",
                     linewidth=1.8,edgecolor=edge,facecolor=fill)
    ax.add_patch(p)
    ax.text(x+w/2,y+h/2,text,ha='center',va='center',fontsize=fs,
            color=tc or C_TEXT,weight=wt)

def arrow(ax,x1,y1,x2,y2,color='#475569',style='-',lw=1.4,label=None,off=(0,0)):
    a=FancyArrowPatch((x1,y1),(x2,y2),arrowstyle='->',mutation_scale=14,
                      color=color,linewidth=lw,linestyle=style)
    ax.add_patch(a)
    if label:
        mx,my=(x1+x2)/2+off[0],(y1+y2)/2+off[1]
        ax.text(mx,my,label,ha='center',va='center',fontsize=8,color=color,
                bbox=dict(boxstyle='round,pad=0.15',fc='white',ec='none',alpha=0.85))

def group(ax,x,y,w,h,title,fill='#f8fafc',edge='#94a3b8',fs=11):
    r=Rectangle((x,y),w,h,linewidth=1.4,edgecolor=edge,facecolor=fill,alpha=0.45)
    ax.add_patch(r)
    ax.text(x+0.15,y+h-0.18,title,ha='left',va='top',fontsize=fs,color=C_TEXT,weight='bold')

# ========== 图 1：系统架构图 ==========
fig,ax=plt.subplots(figsize=(14,9),dpi=150)
ax.set_xlim(0,14); ax.set_ylim(0,9); ax.set_axis_off()
ax.set_title('车载式铁路周边环境巡检设备 — 系统整体架构图',fontsize=15,weight='bold',pad=14)

group(ax,0.3,7.4,13.4,1.3,' 第 1 层：车顶安装（IP65）',fill='#eff6ff',edge=C_PRIMARY)
box(ax,0.6,7.55,2.8,1.0,'蘑菇头 GNSS 天线 ×2\nGPS/PTP 信号',C_ROOF,C_PRIMARY,10,'bold')
box(ax,3.7,7.55,3.0,1.0,'禾赛 XT32\n32线 LiDAR · 64万点/秒',C_ROOF,C_PRIMARY,10,'bold')
box(ax,7.0,7.55,3.0,1.0,'4× 全局快门相机\n640×480 @ 30fps',C_ROOF,C_PRIMARY,10,'bold')
box(ax,10.3,7.55,3.2,1.0,'钢丝减震 ×8\n车顶振动防护',C_DAMP,'#f59e0b',10,'bold')

group(ax,0.3,4.3,13.4,3.0,'️ 第 2 层：铝合金 CNC 机箱（IP65，模块化）',fill='#f0fdf4',edge='#10b981')
box(ax,0.6,5.5,2.6,1.5,' 电源系统\nAC-DC 24V/15A\n+ 24V/10Ah LiFePO4\n+ DC-DC 精简版',C_POWER,'#ef4444',9)
box(ax,3.4,5.5,2.6,1.5,' PCB 自研一体化板 ¥500\n① 电源滤波\n② PPS/PTP 同步 ≤1ms\n③ 4路相机触发',C_CAB,'#10b981',9)
box(ax,6.2,5.5,2.8,1.5,'️ Jetson Orin NX 16GB\n100 TOPS INT8\n2 路千兆网口\n主控（SLAM/AI/Qt）',C_CAB,'#10b981',10,'bold')
box(ax,9.2,5.5,2.0,1.5,' ADIS16477-3 IMU\n0.8/h, ±40g\nSPI 接口',C_CAB,'#10b981',9)
box(ax,11.4,5.5,2.1,1.5,' UMD982 RTK\n双天线\n水平 ≤2cm',C_CAB,'#10b981',9)
box(ax,0.6,4.45,5.4,0.95,' 2×1TB SSD + 2×8TB HDD  ＝  17TB 混合存储',C_CAB,'#10b981',9.5,'bold')
box(ax,6.2,4.45,3.5,0.95,'️ 被动散热片 → CNC 外壳导热（无风扇）',C_CAB,'#10b981',9.5)
box(ax,9.9,4.45,3.6,0.95,' LED 指示灯 + 按键（OLED 已去除）',C_CAB,'#10b981',9.5)

group(ax,0.3,2.3,13.4,1.85,'️ 第 3 层：软件数据流（车内，ROS2 Humble）',fill='#faf5ff',edge=C_AI)
box(ax,0.6,2.5,2.6,1.4,'ROS2 Humble\nPCL 1.12\nOpenCV 4.5',C_SW,C_AI,9.5)
box(ax,3.4,2.5,2.6,1.4,'SLAM\nFAST-LIO2\nLiDAR-IMU 紧耦合',C_SW,C_AI,9.5)
box(ax,6.2,2.5,3.0,1.4,' AI 推理\nYOLOv8x + TensorRT FP16\n单帧 ≤30ms',C_SW,C_AI,10,'bold')
box(ax,9.4,2.5,4.1,1.4,'️ Qt 6 上位机\n实时告警 ≤500ms / 声光 / 数据录制\nCVAT 标注闭环 → 模型迭代',C_SW,C_AI,9.5)

group(ax,0.3,0.6,13.4,1.55,' 第 4 层：AI 多任务异常识别（验收硬指标 mAP ≥ 0.75）',fill='#fffbeb',edge='#f59e0b')
box(ax,0.6,0.75,4.1,1.2,' 异物入侵\n落石 / 行人 / 车辆 / 牲畜\nmAP ≥ 0.80','#fecaca','#dc2626',10,'bold')
box(ax,5.0,0.75,4.1,1.2,' 周边环境变化\n新建筑 / 违建 / 取土 / 植被\nmAP ≥ 0.70',C_DAMP,'#f59e0b',10,'bold')
box(ax,9.4,0.75,4.1,1.2,' 施工作业\n人员 / 机械 / 围挡 / 警示牌\nmAP ≥ 0.75',C_DAMP,'#f59e0b',10,'bold')

arrow(ax,2.0,7.55,2.0,7.05,color=C_PRIMARY,lw=1.6,label='GPS/PTP',off=(0,0.06))
arrow(ax,5.2,7.55,5.2,7.05,color=C_PRIMARY,lw=1.6,label='千兆网',off=(0,0.06))
arrow(ax,8.5,7.55,8.5,7.05,color=C_PRIMARY,lw=1.6,label='千兆网',off=(0,0.06))
arrow(ax,11.9,7.55,11.9,7.05,color='#f59e0b',lw=1.4,style='--',label='车顶减震')
arrow(ax,4.7,6.25,6.2,6.25,color='#475569',lw=1.0,label='PPS/触发')
arrow(ax,7.6,5.5,1.9,3.9,color=C_AI,lw=1.4,style='--',label='数据流')
arrow(ax,3.2,3.2,3.4,3.2,color=C_AI,lw=1.4)
arrow(ax,6.0,3.2,6.2,3.2,color=C_AI,lw=1.4)
arrow(ax,9.2,3.2,9.4,3.2,color=C_AI,lw=1.4)
arrow(ax,7.7,2.5,2.6,1.95,color='#dc2626',lw=1.2,style=':')
arrow(ax,7.7,2.5,7.0,1.95,color='#f59e0b',lw=1.2,style=':')
arrow(ax,7.7,2.5,11.4,1.95,color='#f59e0b',lw=1.2,style=':')

ax.legend(handles=[
    Line2D([0],[0],color=C_PRIMARY,lw=2,label='硬件链路'),
    Line2D([0],[0],color=C_AI,lw=2,label='软件数据流'),
    Line2D([0],[0],color='#dc2626',lw=2,linestyle=':',label='异常告警'),
],loc='lower left',fontsize=9,framealpha=0.9)

plt.tight_layout()
plt.savefig(f"{OUT}/arch_v6.png",dpi=150,bbox_inches='tight',facecolor='white')
plt.close()
print(f"OK: {OUT}/arch_v6.png")

# ========== 图 2：AI 数据流 ==========
fig,ax=plt.subplots(figsize=(14,7),dpi=150)
ax.set_xlim(0,14); ax.set_ylim(0,7); ax.set_axis_off()
ax.set_title('AI 异常识别数据流图（4 路相机 + LiDAR → 多任务检测 → 上位机）',fontsize=15,weight='bold',pad=14)

stages=[
    (' 输入',0.3,5.0,2.4,1.7,'#dbeafe',C_PRIMARY),
    ('️ 处理',3.0,5.0,3.0,1.7,'#f0fdf4','#10b981'),
    (' 多任务分类',6.3,5.0,3.4,1.7,'#f3e8ff',C_AI),
    (' 输出',10.0,5.0,3.7,1.7,'#fef3c7','#f59e0b'),
]
for title,x,y,w,h,fill,edge in stages:
    group(ax,x,y,w,h,title,fill=fill,edge=edge)

box(ax,0.5,5.15,2.1,1.4,'4× 全局快门相机\n640×480 @ 30fps\n1280×720 可选','#dbeafe',C_PRIMARY,9)
box(ax,0.5,3.5,2.1,1.4,'禾赛 XT32 LiDAR\n64万点/秒\n120m 测距','#dbeafe',C_PRIMARY,9)
box(ax,3.2,5.15,2.7,1.4,'ROS2 Humble 话题\n/camera/* + /lidar/points\n/imu + /rtk','#f0fdf4','#10b981',9.5,'bold')
box(ax,3.2,3.5,2.7,1.4,'图像预处理\nOrin NX 实时解码\n640×640 resize','#f0fdf4','#10b981',9.5)
box(ax,3.2,1.85,2.7,1.4,' YOLOv8x 检测\nTensorRT FP16\n单帧 ≤30ms','#f0fdf4','#10b981',10,'bold')
box(ax,6.5,5.15,3.1,1.4,' 异物入侵\n落石 / 行人 / 车辆 / 牲畜\nmAP ≥ 0.80','#fecaca','#dc2626',9.5,'bold')
box(ax,6.5,3.5,3.1,1.4,' 周边环境变化\n新建筑 / 违建 / 取土 / 植被\nmAP ≥ 0.70',C_DAMP,'#f59e0b',9.5,'bold')
box(ax,6.5,1.85,3.1,1.4,' 施工作业\n人员 / 机械 / 围挡 / 警示牌\nmAP ≥ 0.75',C_DAMP,'#f59e0b',9.5,'bold')
box(ax,10.2,5.15,3.4,1.4,' 实时告警\nQt 上位机弹窗\n声光 + 日志\n延迟 ≤500ms','#fef3c7','#f59e0b',9.5,'bold')
box(ax,10.2,3.5,3.4,1.4,' 数据录制\n触发录像 + ROS Bag\n17TB 存储','#fef3c7','#f59e0b',9.5)
box(ax,10.2,1.85,3.4,1.4,'️ CVAT 标注闭环\nAI 数据回灌 → 模型迭代','#fef3c7','#f59e0b',9.5)

arrow(ax,2.6,5.85,3.2,5.85,color=C_PRIMARY,lw=1.6)
arrow(ax,2.6,4.2,3.2,4.2,color=C_PRIMARY,lw=1.6)
arrow(ax,5.9,5.85,6.5,5.85,color='#10b981',lw=1.4)
arrow(ax,5.9,4.2,6.5,4.2,color='#10b981',lw=1.4)
arrow(ax,5.9,2.55,6.5,2.55,color='#10b981',lw=1.4)
arrow(ax,9.6,5.85,10.2,5.85,color='#dc2626',lw=1.4)
arrow(ax,9.6,4.2,10.2,4.2,color='#dc2626',lw=1.4)
arrow(ax,9.6,2.55,10.2,2.55,color='#dc2626',lw=1.4)
arrow(ax,4.55,5.15,4.55,4.9,color='#475569',lw=1.0)
arrow(ax,4.55,3.5,4.55,3.25,color='#475569',lw=1.0)
arrow(ax,11.9,1.85,4.55,1.85,color=C_AI,lw=1.4,style='--',label='模型迭代反馈',off=(0,0.15))

ax.text(7,0.5,' 验收硬指标：单帧推理 ≤30ms · 实时告警 ≤500ms · mAP ≥0.75 · 误报率 ≤5%',
        ha='center',va='center',fontsize=11,color=C_TEXT,weight='bold',
        bbox=dict(boxstyle='round,pad=0.5',fc='#dbeafe',ec=C_PRIMARY,lw=1.2))

plt.tight_layout()
plt.savefig(f"{OUT}/dataflow_v6.png",dpi=150,bbox_inches='tight',facecolor='white')
plt.close()
print(f"OK: {OUT}/dataflow_v6.png")
