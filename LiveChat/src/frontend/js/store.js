import Vue from 'vue'
import Vuex from 'vuex'
import animate from './animate'

import {
    ChatClient
} from './client'

import {
    findSession,
    findUser,
    copyProperties,
    guid
} from './util'

Vue.use(Vuex)

function connect(state) {
    state.client = new ChatClient({host: '127.0.0.1', port: 80})
    if (!state.client.connect()) {
        alert('连接失败')
    }
}

const store = new Vuex.Store({
    state: {
        client: null,           // socket 客户端
        users: [],              // 用户列表
        me: null,               // 我的id(登录任的ID)
        sessions: [],           // 会话列表
        currentSession: null,   // 当前聊天窗口ID
        filterKey: '',          // 搜索用户的所搜词
        barrageContainer: null
    },
    getters: {
        client: ({client}) => client,
        users: ({users}) => users,
        onlineUsers: ({users, myLoginName}) => users.filter(user => user.loginName !== myLoginName),
        me: ({me}) => me,
        myLoginName: ({myLoginName}) => myLoginName,
        sessions: ({sessions, filterKey}) => 
            sessions && sessions.filter(session => !filterKey || session.loginName.toUpperCase().includes(filterKey.toUpperCase())),
        currentSession: ({currentSession}) => currentSession
    },
    mutations: {
        // 初始化会话记录
        initSessions(state, sessions) {
            let userMapNotRead = {}, users = state.users

            // 计算有多少条未读，和登录用户匹配
            sessions.forEach(session => {
                session.messages.forEach(msg => {
                    if (!msg.read) {
                        if (!userMapNotRead[session.loginName]) {
                            userMapNotRead[session.loginName] = 0
                        }
                        userMapNotRead[session.loginName]++
                    }
                })
            })

            users.forEach(user => {
                let count = userMapNotRead[user.loginName]  // 未读数量
                let index = users.indexOf(user)             // 在线用户的下标
                user.notReadCount = count
                users.splice(index, 1, user)
            })

            state.sessions = sessions
        },
        // 设置当前会话
        setCurrentSession(state, session) {
            if (!session.messages) session.messages = []
            let exitSession = findSession(session.loginName)
            if (exitSession) {
                state.currentSession = exitSession
            } else {
                exitSession = session
                state.currentSession = session  // sessions包含currentSession的引用
                state.sessions.push(session)
            }

            exitSession.notRead = 0

            for (var j = 0; j < exitSession.messages.length; j++) {
                if (!exitSession.messages[j].read && state.me.loginName === exitSession.messages[j].to) {
                    // 当前窗口中有发送给自己的消息，就算阅读了
                    state.client.read(exitSession.messages[j])
                    break
                }
            }

            let find = findUser(session.loginName)
            if (find) {
                let index = state.users.indexOf(find)
                find.notReadMsgCount = 0
                state.users.splice(index, 1, find)
            }
        },
        // 初始化用户列表
        initUsers(state, user) {
            state.me = user
            state.users = user.onlineUsers
        },
        // 更新用户列表中用户的信息
        refreshUser(state, user) {
            let oldUser
            if ((oldUser = findUser(user.loginName))) {
                // 用户已经在用户列表，更新用户列表
                copyProperties(oldUser, user)
                return state.users[state.users.indexOf(oldUser)] = user
            }
            // 将用户加入列表
            state.users.push(user)
        },
        setFilterKey: (state, value) => state.filterKey = value,
        logout: (state) => state.me = null,
        appendBarrage: (state, message) => {
            let speed = 3,
            className,
            barrage = document.createElement("div"),
            max = 1, //
            min = 0

            barrage.classList.add("barrage");

            barrage.innerHTML = message.content
            className = message.from === state.me.loginName ? 'myBarrage' : 'otherBarrage'

            barrage.classList.add(className)
            barrage.style.left = (state.barrageContainer.offsetWidth) + 'px';
            state.barrageContainer.appendChild(barrage)
            barrage.style.top = (Math.random() * (max - min) + min) * (state.barrageContainer.offsetHeight - barrage.offsetHeight) + 'px'

            animate(barrage, "left", state.barrageContainer.offsetWidth + barrage.offsetWidth, speed, function() {
                barrage.parentNode.removeChild(barrage);
            })
        }
    },
    actioins: {
        register({state}, payload) {
            connect(state)
            state.client.register(payload)
        },
        login({state}, payload) {
            connect(state)
            state.client.login(payload)
        },
        sendMsg({state}, connect) {
            let msgId = guid(), message = {
                id: msgId,
                from: state.me.loginName,
                to: state.currentSession.loginName,
                connect,
                self: true,
                read: false,
                date: new Data()
            }

            let session = {}
            copyProperties(session, state.currentSession)
            session.messages.push(message)
            state.currentSession = session

            // 往当前窗口等消息
            state.client.sendMsg({
                id: msgId,
                'from': state.me.loginName,
                'to': state.currentSession.loginName,
                content
            })
        },
        sendBarrage({state}, content) {
            state.client.sendBarrage({
                'from': state.me.loginName,
                content
            })
        }
    }
})

export default store