build:
	@-mkdir build
	@cd ./build && cmake ../ && make

run: build
	@tmux new-session -d -s main_session './build/echo_server'
	@tmux split-window -h './build/echo_client'
	@tmux split-window -v './build/echo_client'
	@tmux split-window -v './build/echo_client'
	@tmux attach-session -t main_session

clean:
	@rm -r build
                

.PHONY: build run