package com.odafs.app

import android.content.res.Configuration
import android.os.Bundle
import android.widget.Space
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.animation.animateColorAsState
import androidx.compose.animation.animateContentSize
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.tooling.preview.datasource.LoremIpsum
import androidx.compose.ui.unit.dp
import com.odafs.app.ui.theme.ODAFSTheme

var lorem: String = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque vitae maximus eros, in rhoncus sapien. Aliquam eu sem luctus, viverra odio vitae, pulvinar dolor. Vestibulum eu leo justo. Nam consequat pellentesque nulla ullamcorper maximus. Proin bibendum, lectus nec auctor auctor, mi turpis eleifend nulla, quis aliquam ligula quam at nunc. Nulla facilisi. Vivamus semper nulla vel lectus vestibulum, quis bibendum tortor dictum. Sed quis porttitor libero. Quisque sit amet eros felis. Nunc odio arcu, imperdiet vitae ipsum vel, vulputate rutrum purus."

data class Message(val name: String, val desc: String)

object MessageList {
    val messageList = listOf<Message>(
        Message(name = "nombre 1", desc = lorem),
        Message(name = "nombre 2", desc = lorem),
        Message(name = "nombre 3", desc = lorem)
    )
}

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            ODAFSTheme () {
                Surface (modifier = Modifier.fillMaxSize()) {
                    Column {
                        LazyColumn (modifier = Modifier.padding(all = 10.dp)){
                            items(MessageList.messageList) { message ->
                                Greeting(message = message)
                                Spacer(modifier = Modifier.height(5.dp))
                            }
                        }

                        var counter by remember { mutableIntStateOf(0) }
                        Button(onClick = {counter++}) {
                            Text(text = "button clicks: $counter")
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun Greeting(message: Message) {
    var isExpanded by remember { mutableStateOf(false) }
    val surfaceColor by animateColorAsState(
        targetValue = if (isExpanded) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.surface
    )
    Surface (
        shape = MaterialTheme.shapes.medium,
        shadowElevation = 5.dp,
        color = surfaceColor,
        modifier = Modifier.animateContentSize().padding(1.dp)
    ) {
        Column (modifier = Modifier.clickable {isExpanded = !isExpanded}) {
            Text(
                text = "Hello ${message.name}!",
                style = MaterialTheme.typography.titleLarge,
                modifier = Modifier.padding(all = 6.dp)
            )

            Spacer(modifier = Modifier.height(2.dp))

            Text(
                text = message.desc,
                style = MaterialTheme.typography.titleLarge,
                maxLines = if (isExpanded) Int.MAX_VALUE else 1,
                modifier = Modifier.padding(all = 6.dp)
            )
        }
    }
}

@Preview(
    name = "Dark Mode",
    showBackground = true,
    uiMode = Configuration.UI_MODE_NIGHT_YES
)
@Preview(
    name = "Light Mode",
    showBackground = true,
    uiMode = Configuration.UI_MODE_NIGHT_NO,
)
@Composable
fun PreviewGreeting() {
    Greeting(Message(name = "Pepe", desc = "desc"))
}
