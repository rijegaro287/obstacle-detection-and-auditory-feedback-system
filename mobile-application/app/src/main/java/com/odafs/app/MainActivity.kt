package com.odafs.app

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import com.odafs.app.ui.theme.ODAFSTheme

//var lorem: String = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque vitae maximus eros, in rhoncus sapien. Aliquam eu sem luctus, viverra odio vitae, pulvinar dolor. Vestibulum eu leo justo. Nam consequat pellentesque nulla ullamcorper maximus. Proin bibendum, lectus nec auctor auctor, mi turpis eleifend nulla, quis aliquam ligula quam at nunc. Nulla facilisi. Vivamus semper nulla vel lectus vestibulum, quis bibendum tortor dictum. Sed quis porttitor libero. Quisque sit amet eros felis. Nunc odio arcu, imperdiet vitae ipsum vel, vulputate rutrum purus."
//
//data class Message(val name: String, val desc: String)
//
//object MessageList {
//    val messageList = listOf<Message>(
//        Message(name = "nombre 1", desc = lorem),
//        Message(name = "nombre 2", desc = lorem),
//        Message(name = "nombre 3", desc = lorem)
//    )
//}

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            ODAFSTheme {
                NavigationController()
            }
        }
    }
}

//Surface (modifier = Modifier.fillMaxSize()) {
//    Column {
//        LazyColumn (modifier = Modifier.padding(all = 10.dp)){
//            items(MessageList.messageList) { message ->
//                Greeting(message = message)
//                Spacer(modifier = Modifier.height(5.dp))
//            }
//        }
//
//        var counter by remember { mutableIntStateOf(0) }
//        Button(onClick = {counter++}) {
//            Text(text = "button clicks: $counter")
//        }
//    }
//}
//
//@Composable
//fun Greeting(message: Message) {
//    var isExpanded by remember { mutableStateOf(false) }
//    val surfaceColor by animateColorAsState(
//        targetValue = if (isExpanded) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.surface
//    )
//    Surface (
//        shape = MaterialTheme.shapes.medium,
//        shadowElevation = 5.dp,
//        color = surfaceColor,
//        modifier = Modifier.animateContentSize().padding(1.dp)
//    ) {
//        Column (modifier = Modifier.clickable {isExpanded = !isExpanded}) {
//            Text(
//                text = "Hello ${message.name}!",
//                style = MaterialTheme.typography.titleLarge,
//                modifier = Modifier.padding(all = 6.dp)
//            )
//
//            Spacer(modifier = Modifier.height(2.dp))
//
//            Text(
//                text = message.desc,
//                style = MaterialTheme.typography.titleLarge,
//                maxLines = if (isExpanded) Int.MAX_VALUE else 1,
//                modifier = Modifier.padding(all = 6.dp)
//            )
//        }
//    }
//}
//
//@Preview(
//    name = "Dark Mode",
//    showBackground = true,
//    uiMode = Configuration.UI_MODE_NIGHT_YES
//)
//@Preview(
//    name = "Light Mode",
//    showBackground = true,
//    uiMode = Configuration.UI_MODE_NIGHT_NO,
//)
//@Composable
//fun PreviewGreeting() {
//    Greeting(Message(name = "Pepe", desc = "desc"))
//}
