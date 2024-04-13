<script>
    import {
        mapGetters,
        mapActions
    } from 'vuex'

    import {
        cache
    } from '../js/util'

    export default {
        data() {
            return {
                showLoginForm: true,
                users: {
                    loginName: '',
                    password: '',
                    confirmPassword: '',
                    registerCode: ''
                }
            }
        },
        methods: {
            ...mapActions(['login', 'register']),
            denglu() {
                if (!this.user.loginName) {
                    alert('用户名不能为空')
                    return
                }

                if (!this.user.password) {
                    alert('密码不能为空')
                    return
                }

                this.login(this.user);
            },
            zhuce() {
                if (!this.user.loginName) {
                    alert('用户名不能为空')
                    return
                }

                if (!this.user.password) {
                    alert('密码不能为空')
                    return
                }

                if (!this.user.password) {
                    alert('确认秘密不能为空')
                    return
                }

                if (!this.user.password !== this.user.password) {
                    alert('两次输入秘密不一致')
                    return
                }

                this.register(this.user)
            },
            toggleLogin() {
                this.showLoginForm = !this.showLoginForm
            }
        },
        mounted() {
            var credential = cache('credential')
            if (!credential) {
                return
            }

            if (credential.autoLogin) {
                this.$store.dispatch('login', credential)
            } else {
                this.user.loginName = credential.loginName
                this.user.password = credential.password
            }
        }
    }
</script>
<template>
    <div id="loginPanel">
        <div id="loginForm">
            <div class="login" v-show="showLoginForm">
                <p>
                    <span class="soul-label">
                    账号:
                    </span>
                    <input class="soul-input" type="text" v-model="user.loginName" />
                </p>

                <p>
                    <span class="soul-label">
                    密码:
                    </span>
                    <input class="soul-input" type="text" v-model="user.password" />
                </p>

                <div class="loginButtonPanel">
                    <a class="toggle" @click="toggleLogin">还没注册？</a>
                    <button class="soul-button" @click="denglu">登录</button>
                </div>
            </div>

            <div class="register" v-show="!showLoginForm">
                <p>
                    <span class="soul-label">
                    账号:
                    </span>
                    <input class="soul-input" type="text" v-model="user.loginName"/>
                </p>

                <p>
                    <span class="soul-label">
                    密码:
                    </span>
                    <input class="soul-input" type="text" v-model="user.password"/>
                </p>

                <p>
                    <span class="soul-label">
                    确认密码:
                    </span>
                    <input class="soul-input" type="text" v-model="user.confirmPassword"/>
                </p>

                <p>
                    <span class="soul-label">
                    注册秘钥:
                    </span>
                    <input class="soul-input" type="text" v-model="user.registerCode"/>
                </p>

                <div class="loginButtonPanel">
                    <a class="toggle" @click="toggleLogin">已经注册？</a>
                    <button class="soul-button" @click="zhuce">注册</button>
                </div>
            </div>
        </div>
    </div>
</template>
<style lang="less" scoped>
    #loginPanel {
        display: -webkit-flex;
        display: flex;
        -webkit-align-items: center;
        align-items: center;
        -webkit-justify-content: center;
        justify-content: center;
        width: 100%;
        height: 90%;
        border-radius: 3px;
        background-color: rgb(255, 255, 255);
    }
      
    #loginForm {
        width: 80%;
    }
    
    .loginButtonPanel {
        display: -webkit-flex;
        display: flex;
        -webkit-align-items: center;
        align-items: center;
        -webkit-justify-content: center;
        justify-content: center;
    }
    
    .toggle {
        color: blue;
        &:hover {
            cursor: pointer;
        }
    }
</style>