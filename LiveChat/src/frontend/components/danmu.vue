<template>
    <div class="barrageSendPanel">
        <span class="soul-label" style="color: red">{{error}}</span>
        <input stype="height: 50%;" class="soul-input" type="text", v-model="msg">
        <button class="soul-button" @click="sendBarrage">发送弹幕</button>
    </div>
</template>

<script>
    import animation from '../js/animate'
    import store from '../js/store'
    export default {
        data() {
            return {
                msg: "",
                error: '',
            }
        },
        methods: {
            sendBarrage() {
                if (!this.msg) {
                    return this.error = '弹幕不能为空'
                }

                this.$store.dispatch('sendBarrage', this.msg)
                this.msg = this.error = "";
            }
        },
        mounted() {
            store.state.barrageContainer = document.getElementsByClassName('view')[0];
        }
    }
</script>
<style>
    .barrageSendPanel {
        height: 10%;
        display: -webkit-flex;
        display: flex;
        -webkit-align-items: center;
        align-items: center;
        -webkit-justify-content: center;
        justify-content: center;
    }

    .wrap {
        height: 400px;
        width: 80vw;
        border: 1px solid #000;
    }

    .barrage {
        color: white;
        font-size: 14px;
        font-weight: bold;
        text-shadow: 2px 3px 2px #000;
        position: absolute;
        z-index: 1000;
        border-top-left-radius: 14px;
        border-bottom-left-radius: 14px;
        padding: 6px 20px 6px 20px;
        white-space: nowrap;
    }

    .myBarrage {
        border: 5px solid white;
        background: -webkit-linear-gradient(left, rgba(19, 206, 102, 0.7) 50%, rgba(255, 255, 255, 0.2) 100%);
        background: -o-linear-gradient(right, rgba(19, 206, 102, 0.7) 50%, rgba(255, 255, 255, 0.2) 100%);
        background: -moz-linear-gradient(right, rgba(19, 206, 102, 0.7) 50%, rgba(255, 255, 255, 0.2) 100%);
        background: linear-gradient(to right, rgba(19, 206, 102, 0.7) 50%, rgba(255, 255, 255, 0.2) 100%);
    }

    .otherBarrage {
        background: -webkit-linear-gradient(left, rgba(19, 206, 102, 0.7) 50%, rgba(255, 255, 255, 0.2) 100%);
        background: -o-linear-gradient(right, rgba(19, 206, 102, 0.7) 50%, rgba(255, 255, 255, 0.2) 100%);
        background: -moz-linear-gradient(right, rgba(19, 206, 102, 0.7) 50%, rgba(255, 255, 255, 0.2) 100%);
        background: linear-gradient(to right, rgba(19, 206, 102, 0.7) 50%, rgba(255, 255, 255, 0.2) 100%);
    }

</stype>