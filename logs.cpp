#include "Discord.h"
#include "Logs.h"
#include "skStr.h"

#include <windows.h>
#include <winhttp.h>
#include <string>

const std::string webhook_url = skCrypt("Webhook URL").decrypt(); // CHANGE

void SendLogs(const std::string& Content, const std::string& Username, const std::string& Title, const std::string& Description) {
    DiscordWebhook webhook(webhook_url);

    webhook.setContent(Content);
    webhook.setUsername(Username);
    webhook.setAvatarUrl(skCrypt("https://example.com/avatar.png").decrypt());

    DiscordEmbed embed(Title, Description);
    embed.setColor(0x00FF00);
    embed.setTimestamp();

    webhook.addEmbed(embed);
    webhook.execute();
}
