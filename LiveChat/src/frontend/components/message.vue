<script>
    import {mapGetters} from 'vuex'
    export default {
        computed: {
            ...mapGetters(['me', 'currentSession'])
        },
        filters: {
            // 将日期过滤为 hour:minutes
            time (date) {
                if (typeof date === 'string') {
                    date = new Date(date);
                }
                return date.getHours() + ':' + date.getMinutes();
            },
            readFilter(alreadyRead) {
                return alreadyRead
                    ? '已读'
                    : '未读'
                }
        },
        directives: {
            // 发送消息后滚动到底部
            'scroll-bottom': {
                componentUpdated(el){
                    el.scrollTop = el.scrollHeight - el.clientHeight;
                }
            }
        }
    }
</script>
      
<template>
    <div class="message" v-scroll-bottom>
        <ul v-if="currentSession">
            <li v-for="item in currentSession.messages">
                <p class="time">
                    <span>{{ item.date | time}}</span>
                </p>

                <div class="main" :class="{ self: item.self }">
                    <img class="avatar" width="30" height="30" :src="item.self ? me.img : currentSession.img"/>
                    <div class="text">{{ item.content }}</div>
                    <div v-if="item.self" class="read-text">{{ item.read | readFilter}} </div>
                </div>
            </li>
        </ul>
    </div>
</template>
      
<style lang="less" scoped>
    .message {
        padding: 10px 15px;
        overflow-y: scroll;

        li {
            margin-bottom: 15px;
        }

        .time {
            margin: 7px 0;
            text-align: center;

            > span {
                display: inline-block;
                padding: 0 18px;
                font-size: 12px;
                color: #fff;
                border-radius: 2px;
                background-color: #dcdcdc;
            }
        }

        .avatar {
            float: left;
            margin: 0 10px 0 0;
            border-radius: 3px;
        }

        .text {
            vertical-align: middle;
            display: inline-block;
            position: relative;
            padding: 0 10px;
            max-width: ~'calc(100% - 40px)';
            min-height: 30px;
            line-height: 2.5;
            font-size: 12px;
            text-align: left;
            word-break: break-all;
            background-color: #fafafa;
            border-radius: 4px;

            &:before {
                content: " ";
                position: absolute;
                top: 9px;
                right: 100%;
                border: 6px solid transparent;
                border-right-color: #fafafa;
            }
        }

        .read-text {
            vertical-align: bottom;
            display: inline-block;
            position: relative;
            top:20px;
            left: -5px;
            padding: 0 10px;
            max-width: ~'calc(100% - 40px)';
            min-height: 30px;
            line-height: 2;
            font-size: 10px;
            text-align: left;
            word-break: break-all;
            border-radius: 4px;
        }

        .self {
            text-align: right;

            .avatar {
                float: right;
                margin: 0 0 0 10px;
            }

            .text {
                background-color: #b2e281;

                &:before {
                    right: inherit;
                    left: 100%;
                    border-right-color: transparent;
                    border-left-color: #b2e281;
                }
            }
        }
    }
</style>
      