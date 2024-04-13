<script>
import {mapGetters} from 'vuex'
export default {
    computed: {
        ...mapGetters(['currentSession'])
    },

    data(){
        return {
            content: ''
        }
    },
    methods: {
        onKeyup (e) {
            if (e.keyCode === 13 && this.content.length) {
                this.$store.dispatch('sendMsg', this.content);
                this.content = '';
            }
        }
    }
};

</script>
      
<template>
    <div class="text">
        <textarea :disabled="!currentSession" placeholder="按 Enter 发送" v-model="content" @keyup="onKeyup">
        </textarea>
    </div>
</template>
      
<style lang="less" scoped>
.text {
    height: 160px;
    border-top: solid 1px #ddd;

    textarea {
        padding: 10px;
        height: 100%;
        width: 100%;
        border: none;
        outline: none;
        font-family: "Micrsofot Yahei";
        resize: none;
    }
}
</style>
      