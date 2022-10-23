import logging
import os

import click
from click_repl import repl

from oxen_wallet_cli import context

import pywallet3

def _get_config_dir(options):
    """Return the default config dir for network"""
    return os.path.expanduser(os.path.join('~', '.oxen-wallet', options['network']))

@click.group(invoke_without_command=True)
@click.option('--log-level', type=click.Choice(['error', 'warn', 'info', 'debug']))
@click.option('--network', default='testnet', help='Network: mainnet|testnet|devnet.')
@click.option('--config-dir', '-C', default=None, help='Override config directory.')
@click.option('--oxend-url', default="ipc:///home/sean/.oxen/testnet/oxend.sock", type=str, help='Use the given daemon')
@click.option('--datadir', help='A directory which the wallet will save data')
@click.pass_context
def walletcli(click_ctx, **options):
    """Command line interface for Oxen Wallet CLI."""
    if context.configured:
        # In repl mode run configuration once only
        return

    if options['log_level']:
        py_log_level = {
            'error': logging.ERROR,
            'warn': logging.WARNING,
            'info': logging.INFO,
            'debug': logging.DEBUG,
        }[options['log_level']]

        logging.basicConfig(level=py_log_level)

    if options['config_dir'] is None:
        options['config_dir'] = _get_config_dir(options)
    os.makedirs(options['config_dir'], exist_ok=True)

    if options['datadir'] is None:
        options['datadir'] = os.path.join(options['config_dir'], 'oxen_datadir')
    os.makedirs(options['datadir'], exist_ok=True)

    context.configure(options)

    if click_ctx.invoked_subcommand is None:
        click.echo("Run ':help' for help information, or ':quit' to quit.")
        repl(click_ctx)

@walletcli.command()
def load_test_wallet():
    click.echo("Loading test wallet")
    if context.wallet is not None:
        click.echo("Wallet already loaded")
        return

    spend_priv = "e6c9165356c619a64a0d26fafd99891acccccf8717a8067859d972ecd8bcfc0a"
    spend_pub = "b76f2d7c8a036ff65c564dcb27081c04fe3f2157942e23b0496ca797ba728e4f"
    view_priv = "961d67bb5b3ed1af8678bbfcf621f9c15c2b7bff080892890020bdfd47fe4f0a"
    view_pub = "8a0ebacd613e0b03b8f27bc64bd961ea2ebf4c671c6e7f3268651acf0823fed5"

    keyring = pywallet3.Keyring(spend_priv, spend_pub, view_priv, view_pub, context.options["network"])
    click.echo("Wallet address {} loaded".format(keyring.get_main_address()))
    name = click.prompt("Wallet Name", default="{}-oxen-wallet".format(context.options["network"])).strip()
    context.wallet_core_config.omq_rpc.sockname = name + ".sock";
    context.wallet = pywallet3.Wallet(name, keyring, context.wallet_core_config)

@walletcli.command()
@click.argument('seed_phrase', nargs=25)
@click.argument('seed_phrase_passphrase', default="")
def load_from_seed(seed_phrase, seed_phrase_passphrase):
    click.echo("Loading wallet from seed")
    if context.wallet is not None:
        click.echo("Wallet already loaded")
        return

    seed_phrase_str = ' '.join(seed_phrase)
    keyring = context.keyring_manager.generate_keyring_from_electrum_seed(seed_phrase_str, seed_phrase_passphrase)
    click.echo("Wallet address {} loaded".format(keyring.get_main_address()))
    name = click.prompt("Wallet Name", default="{}-oxen-wallet".format(context.options["network"])).strip()
    context.wallet_core_config.omq_rpc.sockname = name + ".sock";
    context.wallet = pywallet3.Wallet(name, keyring, context.wallet_core_config)

@walletcli.command()
def register_service_node():
    click.echo("Registering Service Node")
    if click.confirm("Would you like to register a service node now"):
        click.echo("")
        name = click.prompt("Enter the wallet address of the operator", default="").strip()
        click.echo("The wallet address to be used is: {}".format(name))
        click.echo("TODO: This function is not yet implemented")

@walletcli.command()
def address():
    # click.echo("Address: {}".format(context.keyring.get_main_address()))
    click.echo("Address: {}".format("TODO sean get the address here"))

@walletcli.command()
def get_balance():
    click.echo("Balance: {}".format(context.wallet.get_balance()))

@walletcli.command()
def get_unlocked_balance():
    click.echo("Unlocked Balance: {}".format(context.wallet.get_unlocked_balance()))

def main():
    walletcli()
