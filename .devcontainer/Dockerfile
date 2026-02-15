FROM espressif/idf:v5.5.2


# 使用 sed 命令替换 sources.list 中的域名为阿里云镜像
RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list \
    && sed -i 's/security.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list

RUN apt-get update \
    && apt-get install -y sudo \
    && apt-get install -y gcc git wget make libncurses-dev flex bison gperf \
    && apt-get install -y libgtk-3-dev clangd 

RUN /opt/esp/python_env/idf5.5_py3.12_env/bin/python -m pip install --upgrade idf-component-manager 

ADD entrypoint.sh /root/entrypoint.sh
RUN chmod +x /root/entrypoint.sh
RUN echo "source /root/entrypoint.sh > /dev/null 2>&1" >> /root/.bashrc

RUN /opt/esp/python_env/idf5.5_py3.12_env/bin/python -m pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple \
    && /opt/esp/python_env/idf5.5_py3.12_env/bin/python -m pip config set global.trusted-host pypi.tuna.tsinghua.edu.cn \
    && /opt/esp/python_env/idf5.5_py3.12_env/bin/python -m pip install Pillow \
    && /opt/esp/python_env/idf5.5_py3.12_env/bin/python -m pip install qoi-conv



ENV LC_ALL=C
ENTRYPOINT [ "/root/entrypoint.sh" ]
