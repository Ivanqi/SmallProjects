<script>
    import {mapGetters} from 'vuex'
    import {mapMutations} from 'vuex'
    export default{
        computed: {
            ...mapGetters(['onlineUsers'])
        },
        methods: {
            ...mapMutations(['setCurrentSession'])
        },
        filters: {
            truncateStr(value){
                if (value.length > 7) {
                    return value.substring(0, 6) + '...'
                }
                return value
        }
        }
    }
</script>

<template>
    
<div style="width: 150px">
    当前登录用户
    <ul>
        <li v-for="item in onlineUsers">
            <img style="width: 20px;height: 20px;display: inline-block;vertical-align: middle" :src="item.img"/>
            <span style="float: right;background-color: yellow" v-if="item.notReadMsgCount">{{item.notReadMsgCount}}未读</span>
            <a @click="setCurrentSession(item)" href="#">{{item.loginName | truncateStr}}</a>
        </li>
    </ul>
</div>
</template>

<style lang="less" scoped>
      
</style>
      