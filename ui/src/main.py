import sys
import os
import subprocess
import warnings
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                           QHBoxLayout, QLabel, QTextEdit, QPushButton,
                           QMessageBox, QFileDialog, QTabWidget)
from PyQt5.QtCore import Qt

# 过滤掉 PyQt5 的弃用警告
warnings.filterwarnings("ignore", category=DeprecationWarning)

class RegexpToDFAWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.current_file = None
        self.initUI()
        
    def initUI(self):
        # 创建主窗口部件和布局
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        layout = QVBoxLayout(main_widget)
        
        # 创建文件操作按钮
        file_btn_layout = QHBoxLayout()
        self.new_btn = QPushButton('新建')
        self.open_btn = QPushButton('打开')
        self.save_btn = QPushButton('保存')
        self.save_as_btn = QPushButton('另存为')
        
        self.new_btn.clicked.connect(self.new_file)
        self.open_btn.clicked.connect(self.open_file)
        self.save_btn.clicked.connect(self.save_file)
        self.save_as_btn.clicked.connect(self.save_as_file)
        
        file_btn_layout.addWidget(self.new_btn)
        file_btn_layout.addWidget(self.open_btn)
        file_btn_layout.addWidget(self.save_btn)
        file_btn_layout.addWidget(self.save_as_btn)
        file_btn_layout.addStretch()
        
        layout.addLayout(file_btn_layout)
        
        # 创建输入部分
        input_label = QLabel('正则表达式:')
        layout.addWidget(input_label)
        
        self.input_text = QTextEdit()
        layout.addWidget(self.input_text)
        
        # 创建转换按钮
        self.convert_btn = QPushButton('转换')
        self.convert_btn.clicked.connect(self.convert_regexp)
        layout.addWidget(self.convert_btn)
        
        # 创建选项卡用于显示不同的转换结果
        self.tab_widget = QTabWidget()
        
        # NFA转换表
        self.nfa_text = QTextEdit()
        self.nfa_text.setReadOnly(True)
        self.tab_widget.addTab(self.nfa_text, 'NFA状态转换表')
        
        # DFA转换表
        self.dfa_text = QTextEdit()
        self.dfa_text.setReadOnly(True)
        self.tab_widget.addTab(self.dfa_text, 'DFA状态转换表')
        
        # 最小化DFA转换表
        self.min_dfa_text = QTextEdit()
        self.min_dfa_text.setReadOnly(True)
        self.tab_widget.addTab(self.min_dfa_text, '最小化DFA状态转换表')
        
        layout.addWidget(self.tab_widget)
        
        # 设置窗口属性
        self.setWindowTitle('正则表达式到DFA转换器')
        self.setGeometry(300, 300, 800, 600)
        
    def new_file(self):
        if self.maybe_save():
            self.input_text.clear()
            self.current_file = None
            self.setWindowTitle('正则表达式到DFA转换器')
            
    def open_file(self):
        if self.maybe_save():
            filename, _ = QFileDialog.getOpenFileName(
                self, '打开文件', '',
                '文本文件 (*.txt);;所有文件 (*.*)')
            if filename:
                try:
                    with open(filename, 'r', encoding='utf-8') as f:
                        self.input_text.setText(f.read())
                    self.current_file = filename
                    self.setWindowTitle(f'正则表达式到DFA转换器 - {filename}')
                except Exception as e:
                    QMessageBox.critical(self, '错误',
                                       f'无法打开文件：\n{str(e)}')
                    
    def save_file(self):
        if self.current_file:
            return self.save_file_to(self.current_file)
        return self.save_as_file()
        
    def save_as_file(self):
        filename, _ = QFileDialog.getSaveFileName(
            self, '保存文件', '',
            '文本文件 (*.txt);;所有文件 (*.*)')
        if filename:
            return self.save_file_to(filename)
        return False
        
    def save_file_to(self, filename):
        try:
            with open(filename, 'w', encoding='utf-8') as f:
                f.write(self.input_text.toPlainText())
            self.current_file = filename
            self.setWindowTitle(f'正则表达式到DFA转换器 - {filename}')
            return True
        except Exception as e:
            QMessageBox.critical(self, '错误',
                               f'无法保存文件：\n{str(e)}')
            return False
            
    def maybe_save(self):
        if not self.input_text.document().isModified():
            return True
            
        ret = QMessageBox.warning(
            self, '正则表达式到DFA转换器',
            '文档已被修改。\n是否保存更改？',
            QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)
            
        if ret == QMessageBox.Save:
            return self.save_file()
        elif ret == QMessageBox.Cancel:
            return False
        return True
        
    def convert_regexp(self):
        regexp = self.input_text.toPlainText().strip()
        if not regexp:
            QMessageBox.warning(self, '警告', '请输入正则表达式！')
            return
            
        # 分别处理每一行正则表达式
        regexps = regexp.split('\n')
        nfa_results = []
        dfa_results = []
        min_dfa_results = []
        
        for i, reg in enumerate(regexps, 1):
            reg = reg.strip()
            if not reg:
                continue
                
            try:
                # 调用C++程序进行转换
                result = subprocess.run(
                    ['regexp_to_dfa.exe', reg],
                    capture_output=True,
                    text=True,
                    encoding='utf-8'
                )
                
                if result.returncode == 0:
                    # 解析输出，分离三种转换表
                    output = result.stdout
                    parts = output.split('\n\n')
                    
                    # 为每个部分添加正则表达式标识
                    header = f'正则表达式 {i}: {reg}\n'
                    
                    # NFA部分
                    nfa_part = next((p for p in parts if 'NFA状态转换表' in p), '')
                    if nfa_part:
                        nfa_results.append(header + nfa_part + '\n')
                    
                    # DFA部分
                    dfa_part = next((p for p in parts if 'DFA状态转换表' in p and '最小化' not in p), '')
                    if dfa_part:
                        dfa_results.append(header + dfa_part + '\n')
                    
                    # 最小化DFA部分
                    min_dfa_part = next((p for p in parts if '最小化DFA状态转换表' in p), '')
                    if min_dfa_part:
                        min_dfa_results.append(header + min_dfa_part + '\n')
                else:
                    error_msg = f'正则表达式 {i}: {reg}\n错误：{result.stderr}\n'
                    nfa_results.append(error_msg)
                    dfa_results.append(error_msg)
                    min_dfa_results.append(error_msg)
                    
            except Exception as e:
                error_msg = f'正则表达式 {i}: {reg}\n程序执行错误：{str(e)}\n'
                nfa_results.append(error_msg)
                dfa_results.append(error_msg)
                min_dfa_results.append(error_msg)
        
        # 更新显示结果
        if nfa_results:
            self.nfa_text.setText('\n'.join(nfa_results))
        if dfa_results:
            self.dfa_text.setText('\n'.join(dfa_results))
        if min_dfa_results:
            self.min_dfa_text.setText('\n'.join(min_dfa_results))
        
def main():
    app = QApplication(sys.argv)
    window = RegexpToDFAWindow()
    window.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main() 