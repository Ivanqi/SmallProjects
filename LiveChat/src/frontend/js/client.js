import store from "./store"

import {
    findSession,
    findUser,
    cache,
    copyProperties
} from './util'

// 注册事件
function registerEvent() {
    this.socket.on("register", callBack)
    this.socket.on("login", callBack)
    this.socket.on("receiveMsg", onReceiveMsg)
    this.socket.on("disconnect", onDisconnect)
    this.socket.on("notifyUserLogin", onNotifyUserLogin)
    this.socket.on("kickOff", onKickOff)
    this.socket.on("receiveRead", onReceiveRead)
    this.socket.on("receiveBarrage", onReceiveBarrage)
}

// 注册回调事件(监听服务端推送事件)
ChatClient.prototype.on = function(event, callBack) {
    this.events[event] = callBack;
    return this;
}

// 与服务器建立webSocket连接
ChatClient.prototype.connect = function(data) {
    if (!("io" in window)) {
        alert('浏览器不支持socket.io')
        return false
    }

    let connectUrl = 'http://' + this.host + ":" + this.port
    this.socket = io.connect(connectUrl)
    registerEvent.call(this)
    return true
}

export function ChatClient({host, port}) {
    this.host = host
    this.port = port
    this.socket = null
    this.events = []

    this.register = function(param) {
        this.socket.emit("register", param)
    }

    this.login = function(param) {
        this.socket.emit("login", param)
    }

    this.sendMsg = function(param) {
        this.socket.emit("sendMsg", param)
    }

    this.sendBarrage = function(param) {
        this.socket.emit("sendBarrage", param)
    }

    this.read = function(param) {
        this.socket.emit("read", param)
    }
}

function callBack({code, rMsg, rData}) {
    if (code === 1) {
        store.commit('initUsers', rData)
        store.commit('initSessions', rData.sessions)
        cache('credential', rData)
    } else {
        alert(rMsg)
    }
}

function onReceiveBarrage(message) {
    store.commit("appendBarrage", message)
}

function onReceiveRead(message) {
    let currentSession = store.state.currentSession, sessions = store.state.sessions

    if (currentSession && currentSession.loginName === message.to) {
        // 往当前对话窗口添加信息
        let session = {};
        copyProperties(session, currentSession);
        session.message.find(msg => msg.id === message.id).read = true; // 标记已读
        return store.state.currentSession = session
    }

    let find = findSession(message.to)
    if (find) {
        // 往当前窗口添加消息
        let ss = {}
        copyProperties(ss, find)
        ss.message.find(msg => msg.id === message.id).read = true // 标记已读
        let index = sessions.indexOf(find)
        sessions.splice(index, 1, ss)
    }
}

function onReceiveMsg(message) {
    let exitSession, sessions = store.state.sessions,
        currentSession = store.state.currentSession,
        users = store.state.users
    
    if (currentSession && currentSession.loginName === message.from) {
        // 往当前对话窗口添加信息
        let session = {}
        copyProperties(session, currentSession)
        session.message.push(message)
        store.state.currentSession = session
        return store.state.client.read(message) // 已读
    }

    let userFrom = findUser(message.from)

    // 往聊天缓存中添加信息
    if (!(exitSession = findSession(message.from))) {
        // 聊天记录里面没有from的session，新建from的session
        exitSession = {
            loginName: message.from,
            img: userFrom.img,
            message: [message]
        }

        sessions.push(exitSession)
    } else {
        // from 的 session加入消息
        exitSession.message.push(message)
    }

    if (!currentSession || (currentSession && currentSession.loginName !== message.from)) {
        // 标记未读的数量
        let user = findUser(message.from)
        let index = users.indexOf(user)
        if (!user.notReadMsgCount) user.notReadMsgCount = 0
        user.notReadMsgCount++
        users.splice(index, 1, user)

        let ss = findSession(message.from)
        index = sessions.indexOf(ss)
        ss.notRead = user.notReadMsgCount
        sessions.splice(index, 1, ss)
    }
}

function onDisconnect(loginName) {
    store.state.users = store.state.users.filter(item => item && item.loginName !== loginName)
}

function onNotifyUserLogin(onlineUser) {
    store.commit('refreshUser', onlineUser)
}

function onKickOff({rData}) {
    store.commit('logout')
    alert(rData)
}