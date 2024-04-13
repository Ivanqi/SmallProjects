<script>
    import {mapGetters} from "vuex"
    import {mapMutations} from "vuex"

    export default {
        computed: {
            ...mapGetters([
                'sessions',
                'currentSession'
            ])
        },
        methods: {
            ...mapMutations([
                'setCurrentSession'
            ])
        },
        filters: {
            truncateStr(value) {
                if (value) {
                    return value.substring(0, 10)
                }
            }
        }
    };
</script>

<template>
    <div class="list">
        <ul>
            <li v-for="(item, index) in sessioins" :class="[{
                active: currentSession
                ? item.loginName === currentSession.loginName
                : index == 0}]" @click="setCurrentSession(item)">
                
                <img class="avatar" width="30" :alt="item.loginName" :src="item.img">
                <p class="name">
                    <span>{{item.loginName | truncateStr}}</span>
                    <span style="float:right; background-color: red; border-radius: 5px; width: 30px; text-align=center" v-if="item.notRead">
                </p>
            </li>
        </ul>
    </div>
</template>

<style scoped lang="less">
    .list {

        li {
            padding: 12px 15px;
            border-bottom: 1px solid #292C33;
            cursor: pointer;
            transition: background-color .1s;

            &:hover {
                background-color: rgba(255, 255, 255, 0.03);
            }

            &.active {
                background-color: rgba(255, 255, 255, 0.5);
            }
        }

        .avatar, .name {
            vertical-align: middle;
        }

        .avatar {
            display: block;
            border-radius: 2px;
        }

        .name {
            display: block;
            margin: 0 0 0 15px;
        }
  }
</style>