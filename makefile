-include ef.gy/base.mk include/ef.gy/base.mk

NAME:=eldritchd

.third-party/libefgy/include/ef.gy/base.mk:
	mkdir .third-party || true
	cd .third-party && git clone git://github.com/ef-gy/libefgy.git

include/ef.gy/base.mk: .third-party/libefgy/include/ef.gy/base.mk
	ln -sfn ../.third-party/libefgy/include/ef.gy include/ef.gy

.third-party/prometheus-client-cpp/include/prometheus/version.h:
	mkdir .third-party || true
	cd .third-party && git clone git://github.com/jyujin/prometheus-client-cpp.git

include/prometheus/version.h: .third-party/prometheus-client-cpp/include/prometheus/version.h
	ln -sfn ../.third-party/prometheus-client-cpp/include/prometheus include/prometheus
